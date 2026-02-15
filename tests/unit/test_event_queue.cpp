// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include "lvkw_event_queue.h"
#include "test_helpers.hpp"

class EventQueueTest : public ::testing::Test {
protected:
    LVKW_Context_Base ctx;
    TrackingAllocator allocator;
    LVKW_EventQueue q;

    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        ctx.prv.alloc_cb = TrackingAllocator::get_allocator();
        ctx.prv.allocator_userdata = &allocator;
        
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
    
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, nullptr, &m1);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, nullptr, &m2);
    
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
    
    LVKW_Event s2 = {};
    s2.mouse_scroll.delta = {0, 2};
    
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, nullptr, &s1);
    lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, nullptr, &s2);
    
    lvkw_event_queue_begin_gather(&q);
    EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
    
    lvkw_event_queue_scan(&q, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* w, const LVKW_Event* e, void* u) {
        EXPECT_EQ(e->mouse_scroll.delta.x, 1);
        EXPECT_EQ(e->mouse_scroll.delta.y, 2);
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
