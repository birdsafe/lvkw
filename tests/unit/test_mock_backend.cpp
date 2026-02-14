// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

class MockBackendTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    ci.allocator = TrackingAllocator::get_allocator();
    ci.userdata = &tracker;
    ASSERT_EQ(lvkw_createContext(&ci, &ctx), LVKW_SUCCESS);
    ASSERT_NE(ctx, nullptr);
  }

  void TearDown() override {
    if (ctx) {
      lvkw_ctx_destroy(ctx);
    }
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }
};

TEST_F(MockBackendTest, ContextAttributes) {
  LVKW_ContextAttributes attrs = {};
  attrs.idle_timeout_ms = 5000;
  ASSERT_EQ(lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs), LVKW_SUCCESS);

  attrs.inhibit_idle = true;
  ASSERT_EQ(lvkw_ctx_update(ctx, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs), LVKW_SUCCESS);
}

TEST_F(MockBackendTest, WindowCreation) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test Window";
  wci.app_id = "test.app";
  wci.attributes.logicalSize = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);
  ASSERT_NE(window, nullptr);

  lvkw_mock_markWindowReady(window);
  // Make window ready
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType, LVKW_Window*, const LVKW_Event*, void*) {}, nullptr);

  LVKW_WindowGeometry geometry;
  ASSERT_EQ(lvkw_wnd_getGeometry(window, &geometry), LVKW_SUCCESS);
  EXPECT_EQ(geometry.pixelSize.x, 800);
  EXPECT_EQ(geometry.pixelSize.y, 600);

  lvkw_wnd_destroy(window);
}

TEST_F(MockBackendTest, EventPushPoll) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test Window";
  wci.attributes.logicalSize = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  // Create a mock event
  LVKW_Event ev = {};

  ev.key.key = LVKW_KEY_A;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;

  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, window, &ev);

  bool received = false;
  lvkw_ctx_syncEvents(ctx, 0);
  ASSERT_EQ(lvkw_ctx_scanEvents(
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

  lvkw_wnd_destroy(window);
}

TEST_F(MockBackendTest, GetMonitorsEmpty) {
  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0);
}

TEST_F(MockBackendTest, AddAndGetMonitor) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Test Monitor", {1280, 720});
  ASSERT_NE(m, nullptr);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);

  LVKW_Monitor* monitors[4];
  count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, monitors, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);
  EXPECT_EQ(monitors[0], m);
  EXPECT_STREQ(monitors[0]->name, "Test Monitor");
  EXPECT_EQ(monitors[0]->logical_size.x, 1280);
  EXPECT_TRUE(monitors[0]->is_primary);
}

TEST_F(MockBackendTest, RemoveMonitor) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Removable", {800, 600});

  // Drain connection event
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void*) {}, nullptr);

  lvkw_mock_removeMonitor(ctx, m);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0);
  
  EXPECT_TRUE(m->flags & LVKW_MONITOR_STATE_LOST);
}

TEST_F(MockBackendTest, GetMonitorModes) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "Monitor", {800, 600});

  LVKW_VideoMode mode1 = {{1920, 1080}, 60000};
  LVKW_VideoMode mode2 = {{2560, 1440}, 144000};
  lvkw_mock_addMonitorMode(ctx, m, mode1);
  lvkw_mock_addMonitorMode(ctx, m, mode2);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitorModes(ctx, m, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  LVKW_VideoMode modes[4];
  count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitorModes(ctx, m, modes, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);
  EXPECT_EQ(modes[0].size.x, 1920);
  EXPECT_EQ(modes[1].size.x, 2560);
  EXPECT_EQ(modes[1].refresh_rate_mhz, 144000);
}

TEST_F(MockBackendTest, MonitorConnectionEvent) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx, "EventMonitor", {1024, 768});

  struct TestUd {
    LVKW_Monitor* expected_monitor;
    bool got_it;
  } ud = {m, false};

  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* userdata) {
        TestUd* t = (TestUd*)userdata;
        EXPECT_TRUE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor, t->expected_monitor);
        EXPECT_STREQ(e->monitor_connection.monitor->name, "EventMonitor");
        t->got_it = true;
      },
      &ud);
  EXPECT_TRUE(ud.got_it);

  lvkw_mock_removeMonitor(ctx, m);

  ud.got_it = false;
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* userdata) {
        TestUd* t = (TestUd*)userdata;
        EXPECT_FALSE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor, t->expected_monitor);
        t->got_it = true;
      },
      &ud);
  EXPECT_TRUE(ud.got_it);
}

TEST_F(MockBackendTest, StringInterning) {
  lvkw_mock_addMonitor(ctx, "Same Name", {800, 600});
  lvkw_mock_addMonitor(ctx, "Same Name", {800, 600});

  LVKW_Monitor* monitors[4];
  uint32_t count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, monitors, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  // Same interned pointer
  EXPECT_EQ(monitors[0]->name, monitors[1]->name);
}

TEST_F(MockBackendTest, Update) {
  LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  lvkw_mock_markWindowReady(window);
  // Make window ready
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event*, void*) {}, nullptr);

  LVKW_WindowAttributes attrs = {};
  attrs.title = "Updated Title";
  attrs.logicalSize = {1280, 720};

  // Update title only
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_TITLE, &attrs), LVKW_SUCCESS);

  // Update size only
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_LOGICAL_SIZE, &attrs), LVKW_SUCCESS);

  // Update both
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_TITLE | LVKW_WND_ATTR_LOGICAL_SIZE, &attrs), LVKW_SUCCESS);

  attrs.fullscreen = true;
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_FULLSCREEN, &attrs), LVKW_SUCCESS);

  attrs.cursor_mode = LVKW_CURSOR_LOCKED;
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs), LVKW_SUCCESS);

  attrs.cursor = nullptr;
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR, &attrs), LVKW_SUCCESS);

  lvkw_wnd_destroy(window);
}

TEST_F(MockBackendTest, GatherScanEvents) {
  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);

  ASSERT_EQ(lvkw_ctx_syncEvents(ctx, 0), LVKW_SUCCESS);

  int count = 0;
  auto cb = [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
    (*(int*)ud)++;
  };

  ASSERT_EQ(lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);

  // Second scan should still find the event
  count = 0;
  ASSERT_EQ(lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);
}

TEST_F(MockBackendTest, EventMasking) {
  // Set mask to ignore keys
  LVKW_ContextAttributes attrs = {};
  attrs.event_mask = (LVKW_EventType)(LVKW_EVENT_TYPE_ALL & ~LVKW_EVENT_TYPE_KEY);
  ASSERT_EQ(lvkw_ctx_update(ctx, LVKW_CTX_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);
  
  int count = 0;
  auto cb = [](LVKW_EventType type, LVKW_Window* window, const LVKW_Event* e, void* ud) {
    (*(int*)ud)++;
  };

  lvkw_ctx_syncEvents(ctx, 0);
  ASSERT_EQ(lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0); // Should be filtered out

  // Allow keys again
  attrs.event_mask = LVKW_EVENT_TYPE_ALL;
  ASSERT_EQ(lvkw_ctx_update(ctx, LVKW_CTX_ATTR_EVENT_MASK, &attrs), LVKW_SUCCESS);

  lvkw_mock_pushEvent(ctx, LVKW_EVENT_TYPE_KEY, nullptr, &ev);
  lvkw_ctx_syncEvents(ctx, 0);
  count = 0;
  ASSERT_EQ(lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, cb, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 1);
}
