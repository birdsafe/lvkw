#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

class MockBackendTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
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
  lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event*, void*) {}, nullptr);

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
  ev.type = LVKW_EVENT_TYPE_KEY;
  ev.window = window;
  ev.key.key = LVKW_KEY_A;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;

  lvkw_mock_pushEvent(ctx, &ev);

  bool received = false;
  ASSERT_EQ(lvkw_ctx_pollEvents(
                ctx, LVKW_EVENT_TYPE_ALL,
                [](const LVKW_Event* e, void* ud) {
                  bool* r = (bool*)ud;
                  // The first event might be WINDOW_READY because
                  // lvkw_window_create pushes it
                  if (e->type == LVKW_EVENT_TYPE_KEY) {
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
  LVKW_MonitorInfo info = {};
  info.id = 1;
  info.name = "Test Monitor";
  info.physical_size = {600, 340};
  info.current_mode = {{1920, 1080}, 60000};
  info.is_primary = true;
  info.scale = 1.0f;

  lvkw_mock_addMonitor(ctx, &info);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);

  LVKW_MonitorInfo monitors[4];
  count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, monitors, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 1);
  EXPECT_EQ(monitors[0].id, 1);
  EXPECT_STREQ(monitors[0].name, "Test Monitor");
  EXPECT_EQ(monitors[0].current_mode.size.x, 1920);
  EXPECT_TRUE(monitors[0].is_primary);
}

TEST_F(MockBackendTest, RemoveMonitor) {
  LVKW_MonitorInfo info = {};
  info.id = 42;
  info.name = "Removable";
  lvkw_mock_addMonitor(ctx, &info);

  // Drain connection event
  lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, [](const LVKW_Event*, void*) {}, nullptr);

  lvkw_mock_removeMonitor(ctx, 42);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, nullptr, &count), LVKW_SUCCESS);
  EXPECT_EQ(count, 0);
}

TEST_F(MockBackendTest, GetMonitorModes) {
  LVKW_MonitorInfo info = {};
  info.id = 1;
  info.name = "Monitor";
  lvkw_mock_addMonitor(ctx, &info);

  LVKW_VideoMode mode1 = {{1920, 1080}, 60000};
  LVKW_VideoMode mode2 = {{2560, 1440}, 144000};
  lvkw_mock_addMonitorMode(ctx, 1, mode1);
  lvkw_mock_addMonitorMode(ctx, 1, mode2);

  uint32_t count = 0;
  ASSERT_EQ(lvkw_ctx_getMonitorModes(ctx, 1, nullptr, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  LVKW_VideoMode modes[4];
  count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitorModes(ctx, 1, modes, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);
  EXPECT_EQ(modes[0].size.x, 1920);
  EXPECT_EQ(modes[1].size.x, 2560);
  EXPECT_EQ(modes[1].refresh_rate_mhz, 144000);
}

TEST_F(MockBackendTest, GetMonitorModesInvalidId) {
  uint32_t count = 0;
  EXPECT_EQ(lvkw_ctx_getMonitorModes(ctx, 999, nullptr, &count), LVKW_ERROR);
}

TEST_F(MockBackendTest, MonitorConnectionEvent) {
  LVKW_MonitorInfo info = {};
  info.id = 5;
  info.name = "EventMonitor";
  info.is_primary = true;
  lvkw_mock_addMonitor(ctx, &info);

  bool got_connected = false;
  lvkw_ctx_pollEvents(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](const LVKW_Event* e, void* ud) {
        bool* flag = (bool*)ud;
        EXPECT_TRUE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor.id, 5);
        EXPECT_TRUE(e->monitor_connection.monitor.is_primary);
        *flag = true;
      },
      &got_connected);
  EXPECT_TRUE(got_connected);

  lvkw_mock_removeMonitor(ctx, 5);

  bool got_disconnected = false;
  lvkw_ctx_pollEvents(
      ctx, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
      [](const LVKW_Event* e, void* ud) {
        bool* flag = (bool*)ud;
        EXPECT_FALSE(e->monitor_connection.connected);
        EXPECT_EQ(e->monitor_connection.monitor.id, 5);
        *flag = true;
      },
      &got_disconnected);
  EXPECT_TRUE(got_disconnected);
}

TEST_F(MockBackendTest, StringInterning) {
  LVKW_MonitorInfo info1 = {};
  info1.id = 1;
  info1.name = "Same Name";
  lvkw_mock_addMonitor(ctx, &info1);

  LVKW_MonitorInfo info2 = {};
  info2.id = 2;
  info2.name = "Same Name";
  lvkw_mock_addMonitor(ctx, &info2);

  LVKW_MonitorInfo monitors[4];
  uint32_t count = 4;
  ASSERT_EQ(lvkw_ctx_getMonitors(ctx, monitors, &count), LVKW_SUCCESS);
  ASSERT_EQ(count, 2);

  // Same interned pointer
  EXPECT_EQ(monitors[0].name, monitors[1].name);
}

TEST_F(MockBackendTest, Update) {
  LVKW_WindowCreateInfo wci = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_ctx_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  lvkw_mock_markWindowReady(window);
  // Make window ready
  lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event*, void*) {}, nullptr);

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

  attrs.cursor_shape = LVKW_CURSOR_SHAPE_CROSSHAIR;
  ASSERT_EQ(lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_SHAPE, &attrs), LVKW_SUCCESS);

  lvkw_wnd_destroy(window);
}