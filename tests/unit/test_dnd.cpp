// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include "lvkw/lvkw.h"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

class DndTest : public ::testing::Test {
protected:
    LVKW_Context* ctx;
    TrackingAllocator tracker;

      void SetUp() override {
        LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
        ci.allocator = TrackingAllocator::get_allocator();
        ci.allocator.userdata = &tracker;
        ASSERT_EQ(lvkw_createContext(&ci, &ctx), LVKW_SUCCESS);
      }
    void TearDown() override {
        if (ctx) lvkw_ctx_destroy(ctx);
        EXPECT_FALSE(tracker.has_leaks());
    }
};

TEST_F(DndTest, BasicFlow) {
    LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
    wci.attributes.accept_dnd = true;
    LVKW_Window* window = nullptr;
    ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

    const char* paths[] = {"file1.txt", "file2.txt"};
    
    LVKW_DndAction action = LVKW_DND_ACTION_COPY;
    void* session_data = nullptr;
    LVKW_DndFeedback feedback = {&action, &session_data};

    // 1. Enter
    {
        LVKW_Event ev = {};
        ev.dnd_hover.entered = true;
        ev.dnd_hover.path_count = 2;
        ev.dnd_hover.paths = paths;
        ev.dnd_hover.feedback = &feedback;
        lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_HOVER, window, &ev);
    }

    // 2. Drop
    {
        LVKW_Event ev = {};

        ev.dnd_drop.path_count = 2;
        ev.dnd_drop.paths = paths;
        ev.dnd_drop.session_userdata = &session_data;
        lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_DROP, window, &ev);
    }

    struct Results {
        bool got_hover = false;
        bool got_drop = false;
        LVKW_DndAction final_action = LVKW_DND_ACTION_NONE;
    } results;

    lvkw_ctx_syncEvents(ctx, 0);
    lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
        auto* r = (Results*)ud;
        if (type == LVKW_EVENT_TYPE_DND_HOVER) {
            EXPECT_TRUE(e->dnd_hover.entered);
            EXPECT_EQ(e->dnd_hover.path_count, 2);
            EXPECT_STREQ(e->dnd_hover.paths[0], "file1.txt");
            EXPECT_EQ(*e->dnd_hover.feedback->action, LVKW_DND_ACTION_COPY); // Default
            *e->dnd_hover.feedback->action = LVKW_DND_ACTION_MOVE; // App wants MOVE
            r->got_hover = true;
        } else if (type == LVKW_EVENT_TYPE_DND_DROP) {
            r->got_drop = true;
        }
    }, &results);

    EXPECT_TRUE(results.got_hover);
    EXPECT_TRUE(results.got_drop);

    lvkw_wnd_destroy(window);
}

TEST_F(DndTest, SessionDataPersistence) {
    LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
    wci.attributes.accept_dnd = true;
    LVKW_Window* window = nullptr;
    ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

    LVKW_DndAction action = LVKW_DND_ACTION_COPY;
    void* session_data = nullptr;
    LVKW_DndFeedback feedback = {&action, &session_data};

    // Push Hover(Enter), Hover(Motion), then Leave
    LVKW_Event ev = {};
    
    ev.dnd_hover.entered = true;
    ev.dnd_hover.feedback = &feedback;
    lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_HOVER, window, &ev);

    ev.dnd_hover.entered = false;
    ev.dnd_hover.feedback = &feedback;
    lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_HOVER, window, &ev);


    ev.dnd_leave.session_userdata = &session_data;
    lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_LEAVE, window, &ev);

    struct State {
        int call_count = 0;
        void* session_ptr = nullptr;
    } test_state;

    lvkw_ctx_syncEvents(ctx, 0);
    lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
        auto* ts = (State*)ud;
        ts->call_count++;

        if (type == LVKW_EVENT_TYPE_DND_HOVER) {
            if (e->dnd_hover.entered) {
                EXPECT_EQ(*e->dnd_hover.feedback->session_userdata, nullptr);
                *e->dnd_hover.feedback->session_userdata = (void*)0xDEADBEEF;
            } else {
                EXPECT_EQ(*e->dnd_hover.feedback->session_userdata, (void*)0xDEADBEEF);
            }
        } else if (type == LVKW_EVENT_TYPE_DND_LEAVE) {
            EXPECT_EQ(*e->dnd_leave.session_userdata, (void*)0xDEADBEEF);
            *e->dnd_leave.session_userdata = nullptr; // Cleanup
        }
    }, &test_state);

    EXPECT_EQ(test_state.call_count, 3);

    lvkw_wnd_destroy(window);
}

TEST_F(DndTest, Rejection) {
    LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
    wci.attributes.accept_dnd = true;
    LVKW_Window* window = nullptr;
    ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

    LVKW_DndAction action = LVKW_DND_ACTION_COPY;
    void* session_data = nullptr;
    LVKW_DndFeedback feedback = {&action, &session_data};

    LVKW_Event ev = {};
    ev.dnd_hover.entered = true;
    ev.dnd_hover.feedback = &feedback;
    lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_DND_HOVER, window, &ev);

    lvkw_ctx_syncEvents(ctx, 0);
    lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window*, const LVKW_Event* e, void*) {
        if (type == LVKW_EVENT_TYPE_DND_HOVER) {
            *e->dnd_hover.feedback->action = LVKW_DND_ACTION_NONE;
        }
    }, nullptr);

    // In a real backend, this would have sent a "reject" to the OS.
    // In mock, we can just verify the state was written if we had a way to peek.
    // Since we don't have a peek, this test mainly ensures no crash and action pointer is valid.

    lvkw_wnd_destroy(window);
}
