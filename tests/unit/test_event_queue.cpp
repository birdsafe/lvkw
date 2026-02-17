// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>
#include "event_queue.h"
#include "test_helpers.hpp"

class EventQueueTest : public ::testing::Test {
protected:
    LVKW_Context_Base ctx;
    TrackingAllocator allocator;
    LVKW_EventQueue q;

    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        ctx.prv.allocator = TrackingAllocator::get_allocator();
        ctx.prv.allocator.userdata = &allocator;
        ctx.prv.event_mask = LVKW_EVENT_TYPE_ALL;
        
        LVKW_EventTuning tuning = {8, 64, 16, 2.0};
        lvkw_event_queue_init(&ctx, &q, tuning);
    }

    void TearDown() override {
        lvkw_event_queue_cleanup(&ctx, &q);
        EXPECT_FALSE(allocator.has_leaks());
    }
};

TEST_F(EventQueueTest, InitState) {
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
    EXPECT_EQ(lvkw_event_queue_get_commit_id(&q), 0u);
}

TEST_F(EventQueueTest, CommitIdIncrementsOnSuccessfulCommit) {
    EXPECT_EQ(lvkw_event_queue_get_commit_id(&q), 0u);

    lvkw_event_queue_begin_gather(&q);
    lvkw_event_queue_note_commit_success(&q);
    EXPECT_EQ(lvkw_event_queue_get_commit_id(&q), 1u);

    lvkw_event_queue_begin_gather(&q);
    lvkw_event_queue_note_commit_success(&q);
    EXPECT_EQ(lvkw_event_queue_get_commit_id(&q), 2u);
}

TEST_F(EventQueueTest, PushGatherScan) {
    LVKW_Event evt = {};
    evt.key.key = LVKW_KEY_A;
    
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &evt);
    
    // Should NOT be visible before gather
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    int call_count = 0;
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        (*(int*)u)++;
        EXPECT_EQ(type, LVKW_EVENT_TYPE_KEY);
        EXPECT_EQ(e->key.key, LVKW_KEY_A);
    }, &call_count);
    
    EXPECT_EQ(call_count, 1);
}

TEST_F(EventQueueTest, DoubleBuffering) {
    LVKW_Event evt1 = {};
    evt1.key.key = LVKW_KEY_1;
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &evt1);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    // Push while "scanning" (simulated)
    LVKW_Event evt2 = {};
    evt2.key.key = LVKW_KEY_2;
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &evt2);
    
    // Scan should only see evt1
    int call_count = 0;
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        (*(int*)u)++;
        EXPECT_EQ(e->key.key, LVKW_KEY_1);
    }, &call_count);
    EXPECT_EQ(call_count, 1);
    
    // Another gather should reveal evt2 and HIDE evt1
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    call_count = 0;
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        (*(int*)u)++;
        EXPECT_EQ(e->key.key, LVKW_KEY_2);
    }, &call_count);
    EXPECT_EQ(call_count, 1);
}

TEST_F(EventQueueTest, MotionCompression) {
    LVKW_Event m1 = {};
    m1.mouse_motion.position = {10, 10};
    m1.mouse_motion.delta = {1, 1};
    
    LVKW_Event m2 = {};
    m2.mouse_motion.position = {20, 20};
    m2.mouse_motion.delta = {2, 2};
    
    lvkw_event_queue_push_compressible(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, nullptr, &m1);
    lvkw_event_queue_push_compressible(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, nullptr, &m2);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        EXPECT_EQ(e->mouse_motion.position.x, 20);
        EXPECT_EQ(e->mouse_motion.delta.x, 3);
    }, nullptr);
}

TEST_F(EventQueueTest, ScrollCompression) {
    LVKW_Event s1 = {};
    s1.mouse_scroll.delta = {1, 0};
    s1.mouse_scroll.steps = {1, 0};
    
    LVKW_Event s2 = {};
    s2.mouse_scroll.delta = {0, 2};
    s2.mouse_scroll.steps = {0, 2};
    
    lvkw_event_queue_push_compressible(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, nullptr, &s1);
    lvkw_event_queue_push_compressible(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, nullptr, &s2);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        EXPECT_EQ(e->mouse_scroll.delta.x, 1);
        EXPECT_EQ(e->mouse_scroll.delta.y, 2);
        EXPECT_EQ(e->mouse_scroll.steps.x, 1);
        EXPECT_EQ(e->mouse_scroll.steps.y, 2);
    }, nullptr);
}

TEST_F(EventQueueTest, CapacityGrowth) {
    // Initial capacity is 8
    for(int i=0; i<10; ++i) {
        LVKW_Event e = {};
        e.key.key = (LVKW_Key)i;
        lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    }
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 10);
}

TEST_F(EventQueueTest, RemoveWindowEvents) {
    LVKW_Window* w1 = (LVKW_Window*)0x1;
    LVKW_Window* w2 = (LVKW_Window*)0x2;
    
    LVKW_Event e = {};
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w1, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w2, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w1, &e);
    
    lvkw_event_queue_remove_window_events(&q, w1);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        EXPECT_EQ((size_t)w, 0x2);
    }, nullptr);
}

TEST_F(EventQueueTest, Flush) {
    LVKW_Event e = {};
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_flush(&q);
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
}

TEST_F(EventQueueTest, DoubleBufferGrowth) {
    // Initial capacity is 8.
    // Fill buffer to trigger growth (8 -> 16)
    for(int i=0; i<10; ++i) {
        LVKW_Event e = {};
        lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    }
    
    // active is now capacity 16. stable is still 8.
    lvkw_event_queue_begin_gather(&q);
    // stable is now capacity 16. active was also resized to 16 during gather.
    
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 10);
    
    // Fill again to trigger another growth (16 -> 32)
    for(int i=0; i<20; ++i) {
        LVKW_Event e = {};
        lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    }
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 20);
}

TEST_F(EventQueueTest, FullQueueEvictsOldestCompressibleBeforeDrop) {
    LVKW_Event e = {};

    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);

    LVKW_Event motion = {};
    motion.mouse_motion.position = {10, 10};
    lvkw_event_queue_push_compressible(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, nullptr, &motion);

    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e);

    EXPECT_EQ(q.active->count, 8u);

    // Queue is full: this key push should evict the oldest compressible event (the motion).
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &e));
    EXPECT_EQ(q.active->count, 8u);

    lvkw_event_queue_begin_gather(&q);

    uint32_t counts[2] = {0, 0};
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window*,
                                                      const LVKW_Event*, void* userdata) {
        auto* c = static_cast<uint32_t*>(userdata);
        if (type == LVKW_EVENT_TYPE_KEY) c[0]++;
        if (type == LVKW_EVENT_TYPE_MOUSE_MOTION) c[1]++;
    }, counts);

    EXPECT_EQ(counts[0], 8u);
    EXPECT_EQ(counts[1], 0u);
}

TEST_F(EventQueueTest, FullQueueDropsWhenNoEvictableAndNoGrowth) {
    LVKW_EventQueue local_q;
    LVKW_EventTuning fixed_tuning = {8, 8, 16, 2.0};
    ASSERT_EQ(lvkw_event_queue_init(&ctx, &local_q, fixed_tuning), LVKW_SUCCESS);

    LVKW_Event e = {};
    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(lvkw_event_queue_push(&ctx, &local_q, LVKW_EVENT_TYPE_KEY, nullptr, &e));
    }

    // No compressible events and no growth budget: this must drop.
    EXPECT_FALSE(lvkw_event_queue_push(&ctx, &local_q, LVKW_EVENT_TYPE_KEY, nullptr, &e));

    lvkw_event_queue_begin_gather(&local_q);
    EXPECT_EQ(lvkw_event_queue_get_count(&local_q), 8u);
    lvkw_event_queue_cleanup(&ctx, &local_q);
}

TEST_F(EventQueueTest, ExternalQueueMultiProducerIntegrity) {
    constexpr uint32_t kProducerCount = 4;
    constexpr uint32_t kPerProducer = 4;
    std::atomic<uint32_t> pushed{0};
    std::vector<std::thread> threads;

    for (uint32_t p = 0; p < kProducerCount; ++p) {
        threads.emplace_back([&, p]() {
            for (uint32_t i = 0; i < kPerProducer; ++i) {
                LVKW_Event evt = {};
                evt.key.key = (LVKW_Key)(100 + (int)(p * kPerProducer + i));
                if (lvkw_event_queue_push_external(&q, LVKW_EVENT_TYPE_USER_0, nullptr, &evt)) {
                    pushed.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto &t : threads) t.join();

    lvkw_event_queue_begin_gather(&q);

    uint32_t scanned = 0;
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_USER_0, [](LVKW_EventType, LVKW_Window*,
                                                         const LVKW_Event*, void* u) {
        (*static_cast<uint32_t*>(u))++;
    }, &scanned);

    EXPECT_EQ(scanned, pushed.load(std::memory_order_relaxed));
}
