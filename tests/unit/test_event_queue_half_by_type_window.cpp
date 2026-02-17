// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include <cstring>

#include "event_queue.h"
#include "test_helpers.hpp"

class EventQueueHalfByTypeWindowTest : public ::testing::Test {
protected:
    LVKW_Context_Base ctx;
    TrackingAllocator allocator;
    LVKW_EventQueue q;

    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        ctx.prv.allocator = TrackingAllocator::get_allocator();
        ctx.prv.allocator.userdata = &allocator;
        ctx.prv.event_mask = LVKW_EVENT_TYPE_ALL;

        LVKW_EventTuning tuning = {8, 8, 16, 1.0};
        lvkw_event_queue_init(&ctx, &q, tuning);
    }

    void TearDown() override {
        lvkw_event_queue_cleanup(&ctx, &q);
        EXPECT_FALSE(allocator.has_leaks());
    }
};

TEST_F(EventQueueHalfByTypeWindowTest, KeepsLatestPerWindowPerTypeOnOverflow) {
    LVKW_Window* w1 = (LVKW_Window*)0x1;
    LVKW_Window* w2 = (LVKW_Window*)0x2;

    LVKW_Event e = {};

    e.mouse_motion.position = {1, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w1, &e));
    e.mouse_motion.position = {2, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w1, &e));

    e.mouse_scroll.delta = {10, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, w1, &e));
    e.mouse_scroll.delta = {20, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, w1, &e));

    e.mouse_motion.position = {3, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w2, &e));
    e.mouse_motion.position = {4, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w2, &e));

    e.mouse_scroll.delta = {30, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, w2, &e));
    e.mouse_scroll.delta = {40, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_SCROLL, w2, &e));

    EXPECT_EQ(q.active->count, 8u);

    LVKW_Event key = {};
    key.key.key = LVKW_KEY_A;
    key.key.state = LVKW_BUTTON_STATE_PRESSED;
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, nullptr, &key));

    bool saw_w1_motion_latest = false;
    bool saw_w1_scroll_latest = false;
    bool saw_w2_motion_latest = false;
    bool saw_w2_scroll_latest = false;

    for (uint32_t i = 0; i < q.active->count; ++i) {
        if (q.active->windows[i] == w1 && q.active->types[i] == LVKW_EVENT_TYPE_MOUSE_MOTION &&
            q.active->payloads[i].mouse_motion.position.x == 2) {
            saw_w1_motion_latest = true;
        }
        if (q.active->windows[i] == w1 && q.active->types[i] == LVKW_EVENT_TYPE_MOUSE_SCROLL &&
            q.active->payloads[i].mouse_scroll.delta.x == 20) {
            saw_w1_scroll_latest = true;
        }
        if (q.active->windows[i] == w2 && q.active->types[i] == LVKW_EVENT_TYPE_MOUSE_MOTION &&
            q.active->payloads[i].mouse_motion.position.x == 4) {
            saw_w2_motion_latest = true;
        }
        if (q.active->windows[i] == w2 && q.active->types[i] == LVKW_EVENT_TYPE_MOUSE_SCROLL &&
            q.active->payloads[i].mouse_scroll.delta.x == 40) {
            saw_w2_scroll_latest = true;
        }
    }

    EXPECT_TRUE(saw_w1_motion_latest);
    EXPECT_TRUE(saw_w1_scroll_latest);
    EXPECT_TRUE(saw_w2_motion_latest);
    EXPECT_TRUE(saw_w2_scroll_latest);
}

TEST_F(EventQueueHalfByTypeWindowTest, NeverEvictsNonCompressibleEvents) {
    LVKW_Window* w = (LVKW_Window*)0x1;

    LVKW_Event key = {};
    key.key.key = LVKW_KEY_A;
    key.key.state = LVKW_BUTTON_STATE_PRESSED;

    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w, &key));
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w, &key));

    LVKW_Event motion = {};
    motion.mouse_motion.position = {1, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));
    motion.mouse_motion.position = {2, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));
    motion.mouse_motion.position = {3, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));
    motion.mouse_motion.position = {4, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));
    motion.mouse_motion.position = {5, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));
    motion.mouse_motion.position = {6, 0};
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_MOUSE_MOTION, w, &motion));

    EXPECT_EQ(q.active->count, 8u);

    LVKW_Event extra_key = {};
    extra_key.key.key = LVKW_KEY_B;
    extra_key.key.state = LVKW_BUTTON_STATE_PRESSED;
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, LVKW_EVENT_TYPE_KEY, w, &extra_key));

    uint32_t key_count = 0;
    for (uint32_t i = 0; i < q.active->count; ++i) {
        if (q.active->types[i] == LVKW_EVENT_TYPE_KEY) {
            key_count++;
        }
    }

    EXPECT_EQ(key_count, 3u);
}
