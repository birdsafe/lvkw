// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

static LVKW_Status sync_events(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_Status status = lvkw_events_pump(ctx, timeout_ms);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_events_commit(ctx);
}

class CommonApiContractTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    ci.allocator = TrackingAllocator::get_allocator();
    ci.allocator.userdata = &tracker;
    ASSERT_EQ(lvkw_context_create(&ci, &ctx), LVKW_SUCCESS);
    ASSERT_NE(ctx, nullptr);
  }

  void TearDown() override {
    if (ctx) {
      lvkw_context_destroy(ctx);
    }
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }
};

TEST_F(CommonApiContractTest, ContextAttributes) {
  LVKW_ContextAttributes attrs = {};
  attrs.inhibit_idle = true;
  ASSERT_EQ(lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_INHIBIT_IDLE, &attrs), LVKW_SUCCESS);
}

TEST_F(CommonApiContractTest, WindowCreation) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test Window";
  wci.app_id = "test.app";
  wci.attributes.logical_size = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_display_createWindow(ctx, &wci, &window), LVKW_SUCCESS);
  ASSERT_NE(window, nullptr);

  lvkw_mock_markWindowReady(window);
  // Make window ready
  sync_events(ctx, 0);
  lvkw_events_scan(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType, LVKW_Window*, const LVKW_Event*, void*) {}, nullptr);

  LVKW_WindowGeometry geometry;
  ASSERT_EQ(lvkw_display_getWindowGeometry(window, &geometry), LVKW_SUCCESS);
  EXPECT_EQ(geometry.pixel_size.x, 800);
  EXPECT_EQ(geometry.pixel_size.y, 600);

  lvkw_display_destroyWindow(window);
}

TEST_F(CommonApiContractTest, EventPushPoll) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test Window";
  wci.attributes.logical_size = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_display_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  // Create a mock event
  LVKW_Event ev = {};

  ev.key.key = LVKW_KEY_A;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;

  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, window, &ev);

  bool received = false;
  sync_events(ctx, 0);
  ASSERT_EQ(lvkw_events_scan(
                ctx, LVKW_EVENT_TYPE_ALL,
                [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
                  bool* r = (bool*)ud;
                  // The first event might be WINDOW_READY because
                  // lvkw_window_create pushes it
                  if (type == LVKW_EVENT_TYPE_KEY) {
                    EXPECT_EQ(e->key.key, LVKW_KEY_A);
                    *r = true;
                  }
                },
                &received),
            LVKW_SUCCESS);

  EXPECT_TRUE(received);

  lvkw_display_destroyWindow(window);
}

TEST_F(CommonApiContractTest, PumpThenCommitPublishesSnapshot) {
  LVKW_Event ev = {};
  ASSERT_EQ(lvkw_events_post(ctx, LVKW_EVENT_TYPE_USER_0, nullptr, &ev), LVKW_SUCCESS);

  bool saw_user_event = false;
  ASSERT_EQ(lvkw_events_pump(ctx, 0), LVKW_SUCCESS);
  ASSERT_EQ(lvkw_events_scan(
                ctx, LVKW_EVENT_TYPE_USER_0,
                [](LVKW_EventType, LVKW_Window *, const LVKW_Event *, void *ud) {
                  *static_cast<bool *>(ud) = true;
                },
                &saw_user_event),
            LVKW_SUCCESS);
  EXPECT_FALSE(saw_user_event);

  ASSERT_EQ(lvkw_events_commit(ctx), LVKW_SUCCESS);
  ASSERT_EQ(lvkw_events_scan(
                ctx, LVKW_EVENT_TYPE_USER_0,
                [](LVKW_EventType, LVKW_Window *, const LVKW_Event *, void *ud) {
                  *static_cast<bool *>(ud) = true;
                },
                &saw_user_event),
            LVKW_SUCCESS);
  EXPECT_TRUE(saw_user_event);
}

TEST_F(CommonApiContractTest, PostedEventSurvivesMaskChangeBeforeCommit) {
  LVKW_ContextAttributes attrs = {};
  attrs.event_mask = LVKW_EVENT_TYPE_ALL;
  ASSERT_EQ(lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  LVKW_Event ev = {};
  ASSERT_EQ(lvkw_events_post(ctx, LVKW_EVENT_TYPE_USER_0, nullptr, &ev), LVKW_SUCCESS);

  attrs.event_mask = (LVKW_EventType)(LVKW_EVENT_TYPE_ALL & ~LVKW_EVENT_TYPE_USER_0);
  ASSERT_EQ(lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  bool saw_user_event = false;
  ASSERT_EQ(sync_events(ctx, 0), LVKW_SUCCESS);
  ASSERT_EQ(lvkw_events_scan(
                ctx, LVKW_EVENT_TYPE_USER_0,
                [](LVKW_EventType, LVKW_Window *, const LVKW_Event *, void *ud) {
                  *static_cast<bool *>(ud) = true;
                },
                &saw_user_event),
            LVKW_SUCCESS);
  EXPECT_TRUE(saw_user_event);
}

TEST_F(CommonApiContractTest, GetMonitorsEmpty) {
  uint32_t count = 0;
  ASSERT_EQ(lvkw_display_listMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0);
}

TEST_F(CommonApiContractTest, AddAndGetMonitor) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Test Monitor", {1280, 720});
  ASSERT_NE(m, nullptr);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_display_listMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);

  LVKW_MonitorRef* refs[4];
  count = 4;
  ASSERT_EQ(lvkw_display_listMonitors(ctx, refs, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);
  EXPECT_EQ((LVKW_MonitorRef *)m, refs[0]);
  LVKW_Monitor *monitor = nullptr;
  ASSERT_EQ(lvkw_display_createMonitor(refs[0], &monitor), LVKW_SUCCESS);
  EXPECT_STREQ(monitor->name, "Test Monitor");
  EXPECT_EQ(monitor->logical_size.x, 1280);
  EXPECT_TRUE(monitor->is_primary);
  lvkw_display_destroyMonitor(monitor);
}

TEST_F(CommonApiContractTest, RemoveMonitor) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Removable", {800, 600});

  // Drain connection event
  sync_events(ctx, 0);
  lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void*) {}, nullptr);

  lvkw_mock_removeMonitor(ctx, m);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_display_listMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0);
  
  EXPECT_TRUE(m->flags & LVKW_MONITOR_STATE_LOST);
}

TEST_F(CommonApiContractTest, GetMonitorModes) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Monitor", {800, 600});

  LVKW_VideoMode mode1 = {{1920, 1080}, 60000};
  LVKW_VideoMode mode2 = {{2560, 1440}, 144000};
  lvkw_mock_addMonitorMode(ctx, m, mode1);
  lvkw_mock_addMonitorMode(ctx, m, mode2);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_display_listMonitorModes(ctx, m, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  LVKW_VideoMode modes[4];
  count = 4;
  ASSERT_EQ(lvkw_display_listMonitorModes(ctx, m, modes, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);
  EXPECT_EQ(modes[0].size.x, 1920);
  EXPECT_EQ(modes[1].size.x, 2560);
  EXPECT_EQ(modes[1].refresh_rate_mhz, 144000);
}

TEST_F(CommonApiContractTest, MonitorConnectionEvent) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "EventMonitor", {1024, 768});

  struct TestUd {
    LVKW_MonitorRef* expected_monitor;
    bool got_it;
  } ud = {(LVKW_MonitorRef *)m, false};

  sync_events(ctx, 0);
  lvkw_events_scan(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* userdata) {
        TestUd* t = (TestUd*)userdata;
        EXPECT_TRUE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor_ref, t->expected_monitor);
        LVKW_Monitor *monitor = nullptr;
        ASSERT_EQ(lvkw_display_createMonitor(e->monitor_connection.monitor_ref, &monitor), LVKW_SUCCESS);
        EXPECT_STREQ(monitor->name, "EventMonitor");
        lvkw_display_destroyMonitor(monitor);
        t->got_it = true;
      },
      &ud);
  EXPECT_TRUE(ud.got_it);

  lvkw_mock_removeMonitor(ctx, m);

  ud.got_it = false;
  sync_events(ctx, 0);
  lvkw_events_scan(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* userdata) {
        TestUd* t = (TestUd*)userdata;
        EXPECT_FALSE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor_ref, t->expected_monitor);
        t->got_it = true;
      },
      &ud);
  EXPECT_TRUE(ud.got_it);
}

TEST_F(CommonApiContractTest, StringInterning) {
  lvkw_mock_addMonitor(ctx, "Same Name", {800, 600});
  lvkw_mock_addMonitor(ctx, "Same Name", {800, 600});

  LVKW_MonitorRef* refs[4];
  uint32_t count = 4;
  ASSERT_EQ(lvkw_display_listMonitors(ctx, refs, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  LVKW_Monitor *monitor0 = nullptr;
  LVKW_Monitor *monitor1 = nullptr;
  ASSERT_EQ(lvkw_display_createMonitor(refs[0], &monitor0), LVKW_SUCCESS);
  ASSERT_EQ(lvkw_display_createMonitor(refs[1], &monitor1), LVKW_SUCCESS);
  // Same interned pointer
  EXPECT_EQ(monitor0->name, monitor1->name);
  lvkw_display_destroyMonitor(monitor0);
  lvkw_display_destroyMonitor(monitor1);
}

TEST_F(CommonApiContractTest, Update) {
  LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_display_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  lvkw_mock_markWindowReady(window);
  // Make window ready
  sync_events(ctx, 0);
  lvkw_events_scan(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event*, void*) {}, nullptr);

  LVKW_WindowAttributes attrs = {};
  attrs.title = "Updated Title";
  attrs.logical_size = {1280, 720};

  // Update title only
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_TITLE, &attrs), LVKW_SUCCESS);

  // Update size only
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_LOGICAL_SIZE, &attrs), LVKW_SUCCESS);

  // Update both
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_TITLE | LVKW_WINDOW_ATTR_LOGICAL_SIZE, &attrs), LVKW_SUCCESS);

  attrs.fullscreen = true;
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_FULLSCREEN, &attrs), LVKW_SUCCESS);

  attrs.cursor_mode = LVKW_CURSOR_LOCKED;
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_CURSOR_MODE, &attrs), LVKW_SUCCESS);

  attrs.cursor = nullptr;
  ASSERT_EQ(lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_CURSOR, &attrs), LVKW_SUCCESS);

  lvkw_display_destroyWindow(window);
}

TEST_F(CommonApiContractTest, GatherScanEvents) {
  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);

  ASSERT_EQ(sync_events(ctx, 0), LVKW_SUCCESS);

  int count = 0;
  auto cb = [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
    (*(int*)ud)++;
  };

  ASSERT_EQ(lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);

  // Second scan should still find the event
  count = 0;
  ASSERT_EQ(lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);
}

TEST_F(CommonApiContractTest, EventMasking) {
  // Set mask to ignore keys
  LVKW_ContextAttributes attrs = {};
  attrs.event_mask = (LVKW_EventType)(LVKW_EVENT_TYPE_ALL & ~LVKW_EVENT_TYPE_KEY);
  ASSERT_EQ(lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);
  
  int count = 0;
  auto cb = [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
    (*(int*)ud)++;
  };

  sync_events(ctx, 0);
  ASSERT_EQ(lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0); // Should be filtered out

  // Allow keys again
  attrs.event_mask = LVKW_EVENT_TYPE_ALL;
  ASSERT_EQ(lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);
  sync_events(ctx, 0);
  count = 0;
  ASSERT_EQ(lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);
}
