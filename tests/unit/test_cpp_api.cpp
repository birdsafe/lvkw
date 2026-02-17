// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.hpp"
#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"
#include "test_helpers.hpp"

static void syncEvents(lvkw::Context &ctx, uint32_t timeout_ms = 0) {
  lvkw::pumpEvents(ctx, timeout_ms);
  lvkw::commitEvents(ctx);
}

class CppApiTest : public ::testing::Test {
 protected:
  std::unique_ptr<lvkw::Context> ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    ci.allocator = TrackingAllocator::get_allocator();
    ci.allocator.userdata = &tracker;
    ctx = std::make_unique<lvkw::Context>(ci);
  }

  void TearDown() override {
    ctx.reset();
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }
};

TEST_F(CppApiTest, ExceptionOnFailedCreation) {
  LVKW_ContextCreateInfo ci = {};
  // Forced failure allocator
  ci.allocator.alloc_cb = [](size_t, void*) -> void* { return nullptr; };
  ci.allocator.free_cb = [](void*, void*) {};

  try {
    lvkw::Context context(ci);
    FAIL() << "Expected lvkw::Exception";
  } catch (const lvkw::Exception& e) {
    EXPECT_EQ(e.status(), LVKW_ERROR);
  } catch (...) {
    FAIL() << "Expected lvkw::Exception";
  }
}

TEST_F(CppApiTest, WindowDestructorReleasesMemory) {
  size_t initial_allocs = tracker.active_allocations();
  {
    LVKW_WindowCreateInfo wci = {};
    wci.attributes.title = "Scoped Window";
    wci.app_id = "scoped.app";
    wci.attributes.logicalSize = {640, 480};
    lvkw::Window window = ctx->createWindow(wci);

    EXPECT_GT(tracker.active_allocations(), initial_allocs);
  }
  // With pre-allocated queue, the count should return exactly to initial_allocs
  EXPECT_EQ(tracker.active_allocations(), initial_allocs);
}

TEST_F(CppApiTest, ContextMove) {
  lvkw::Context other = std::move(*ctx);
  EXPECT_NE(other.get(), nullptr);
  EXPECT_EQ(ctx->get(), nullptr);

  // Move back to avoid double free or issues if TearDown uses ctx
  *ctx = std::move(other);
}

TEST_F(CppApiTest, WindowCreation) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);
  EXPECT_NE(window.get(), nullptr);

  lvkw_mock_markWindowReady(window.get());
  // Make window ready
  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType, LVKW_Window*, const LVKW_Event&) {});

  EXPECT_EQ(window.getGeometry().pixelSize.x, 1024);
  EXPECT_EQ(window.getGeometry().pixelSize.y, 768);
}

TEST_F(CppApiTest, WindowAttributes) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Attributes Test";
  wci.attributes.logicalSize = {800, 600};

  lvkw::Window window = ctx->createWindow(wci);
  lvkw_mock_markWindowReady(window.get());
  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](LVKW_EventType, LVKW_Window*, const LVKW_Event&) {});

  window.setTitle("New Title");
  window.setSize({1280, 720});

  EXPECT_EQ(window.getGeometry().pixelSize.x, 1280);
}

TEST_F(CppApiTest, ContextAttributes) {
  ctx->setIdleInhibition(true);
}

TEST_F(CppApiTest, EventVisitor) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

  lvkw_mock_markWindowReady(window.get());

  // Push a mock keyboard event
  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_ESCAPE;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_KEY, window.get(), &ev);

  bool received_key = false;
  bool received_ready = false;

  syncEvents(*ctx);
  lvkw::scanEvents(
      *ctx,
      [&](lvkw::KeyboardEvent e) {
        EXPECT_EQ(e->key, LVKW_KEY_ESCAPE);
        received_key = true;
      },
      [&](lvkw::WindowReadyEvent e) {
        EXPECT_EQ(e.window, window.get());
        received_ready = true;
      });

  EXPECT_TRUE(received_key);
  EXPECT_TRUE(received_ready);
}

TEST_F(CppApiTest, EventCallback) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

  LVKW_Event ev = {};
  ev.mouse_button.button = LVKW_MOUSE_BUTTON_LEFT;
  ev.mouse_button.state = LVKW_BUTTON_STATE_PRESSED;
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_MOUSE_BUTTON, window.get(), &ev);

  bool received = false;
  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, LVKW_EVENT_TYPE_MOUSE_BUTTON, [&](LVKW_EventType type, LVKW_Window* w, const LVKW_Event& e) {
    EXPECT_EQ(type, LVKW_EVENT_TYPE_MOUSE_BUTTON);
    EXPECT_EQ(w, window.get());
    EXPECT_EQ(e.mouse_button.button, LVKW_MOUSE_BUTTON_LEFT);
    received = true;
  });

  EXPECT_TRUE(received);
}

TEST_F(CppApiTest, PollEventsWithMask) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_SPACE;
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_KEY, window.get(), &ev);

  bool received = false;
  lvkw::pollEvents(*ctx, LVKW_EVENT_TYPE_KEY, [&](LVKW_EventType type, LVKW_Window*, const LVKW_Event& e) {
    EXPECT_EQ(type, LVKW_EVENT_TYPE_KEY);
    EXPECT_EQ(e.key.key, LVKW_KEY_SPACE);
    received = true;
  });

  EXPECT_TRUE(received);
}

TEST_F(CppApiTest, OverloadsUtility) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

  LVKW_Event ev = {};
  ev.resized.geometry.logicalSize = {1280, 720};
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_WINDOW_RESIZED, window.get(), &ev);

  bool resized = false;
  auto visitor = lvkw::overloads{
      [&](lvkw::WindowResizedEvent e) {
        EXPECT_EQ(e->geometry.logicalSize.x, 1280);
        resized = true;
      },
      [](auto&&) {}  // catch-all
  };

  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, visitor);
  EXPECT_TRUE(resized);
}

TEST_F(CppApiTest, GetMonitorsEmpty) {
  auto monitors = ctx->getMonitors();
  EXPECT_TRUE(monitors.empty());
}

TEST_F(CppApiTest, GetMonitorsWithMock) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx->get(), "C++ Test Monitor", {1280, 720});
  ASSERT_NE(m, nullptr);

  auto monitors = ctx->getMonitors();
  ASSERT_EQ(monitors.size(), 1);
  EXPECT_EQ(monitors[0], (LVKW_MonitorRef *)m);
  LVKW_Monitor *monitor = ctx->createMonitor(monitors[0]);
  EXPECT_STREQ(monitor->name, "C++ Test Monitor");
  EXPECT_FLOAT_EQ(monitor->scale, 1.0f);
  lvkw_display_destroyMonitor(monitor);
}

TEST_F(CppApiTest, GetMonitorModes) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx->get(), "Mode Monitor", {800, 600});

  LVKW_VideoMode mode = {{3840, 2160}, 60000};
  lvkw_mock_addMonitorMode(ctx->get(), m, mode);

  auto modes = ctx->getMonitorModes(m);
  ASSERT_EQ(modes.size(), 1);
  EXPECT_EQ(modes[0].size.x, 3840);
  EXPECT_EQ(modes[0].refresh_rate_mhz, 60000);
}

TEST_F(CppApiTest, MonitorConnectionEventVisitor) {
  LVKW_Monitor *m = lvkw_mock_addMonitor(ctx->get(), "Event Monitor", {800, 600});

  bool got_event = false;
  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, [&](lvkw::MonitorConnectionEvent e) {
    EXPECT_TRUE(e->connected);
    EXPECT_EQ(e->monitor_ref, (LVKW_MonitorRef *)m);
    got_event = true;
  });

  EXPECT_TRUE(got_event);
}

TEST_F(CppApiTest, PartialVisitorIgnoresUnmasked) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};
  lvkw::Window window = ctx->createWindow(wci);

  // Push two events: one handled by visitor, one NOT
  LVKW_Event ev_key = {};
  ev_key.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_KEY, window.get(), &ev_key);

  LVKW_Event ev_motion = {};
  ev_motion.mouse_motion.position.x = 42;
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_MOUSE_MOTION, window.get(), &ev_motion);

  int key_calls = 0;
  int motion_calls = 0;

  // Visitor only handles keys. This infers mask=KEY.
  syncEvents(*ctx);
  lvkw::scanEvents(*ctx, [&](lvkw::KeyboardEvent) { key_calls++; });

  EXPECT_EQ(key_calls, 1);
  EXPECT_EQ(motion_calls, 0);

  // Verify motion event IS still there if we scan for it
  lvkw::scanEvents(*ctx, [&](lvkw::MouseMotionEvent e) {
    motion_calls++;
  });
  EXPECT_EQ(motion_calls, 1);
}
#ifdef LVKW_ENABLE_CONTROLLER
TEST_F(CppApiTest, ControllerHaptics) {
  uint32_t count = 0;
  ASSERT_EQ(lvkw_input_listControllers(ctx->get(), nullptr, &count), LVKW_SUCCESS);
  ASSERT_GT(count, 0u);
  std::vector<LVKW_ControllerRef *> refs(count);
  ASSERT_EQ(lvkw_input_listControllers(ctx->get(), refs.data(), &count), LVKW_SUCCESS);
  LVKW_Controller *ctrl = nullptr;
  ASSERT_EQ(lvkw_input_createController(refs[0], &ctrl), LVKW_SUCCESS);

  EXPECT_EQ(ctrl->haptic_count, (uint32_t)LVKW_CTRL_HAPTIC_STANDARD_COUNT);
  ASSERT_NE(ctrl->haptic_channels, nullptr);
  EXPECT_STREQ(ctrl->haptic_channels[0].name, "Mock Low Frequency");

  const LVKW_Scalar levels[] = {0.1f, 0.2f, 0.3f, 0.4f};
  ASSERT_EQ(lvkw_input_setControllerHapticLevels(ctrl, 0, 4, levels), LVKW_SUCCESS);

  LVKW_Controller_Mock *mock_ctrl = (LVKW_Controller_Mock *)ctrl;
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[0], 0.1f);
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[1], 0.2f);
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[2], 0.3f);
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[3], 0.4f);

  const LVKW_Scalar rumble[] = {0.9f, 0.8f};
  ASSERT_EQ(lvkw_input_setControllerHapticLevels(ctrl, LVKW_CTRL_HAPTIC_LOW_FREQ, 2, rumble),
            LVKW_SUCCESS);
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[0], 0.9f);
  EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[1], 0.8f);
  lvkw_input_destroyController(ctrl);
}
#endif

TEST_F(CppApiTest, Metrics) {
#ifndef LVKW_GATHER_METRICS
  GTEST_SKIP() << "Metrics gathering is disabled";
#endif

  auto tel = ctx->getMetrics<LVKW_EventMetrics>();
  EXPECT_EQ(tel.peak_count, 0);

  LVKW_Event ev = {};
  ev.key.key = LVKW_KEY_A;

  /* TODO: Telemtry is now only gathered on a flush, it needs to be exposed in the mock 
  lvkw_mock_pushEvent(ctx->get(), LVKW_EVENT_TYPE_KEY, nullptr, &ev);
  lvkw_mock_
  syncEvents(*ctx);

  tel = ctx->getMetrics<LVKW_EventMetrics>();
  EXPECT_EQ(tel.peak_count, 1);

  tel = ctx->getMetrics<LVKW_EventMetrics>(true); // reset
  EXPECT_EQ(tel.peak_count, 1);

  tel = ctx->getMetrics<LVKW_EventMetrics>();
  EXPECT_EQ(tel.peak_count, 1); // current count is 1
  */
}
