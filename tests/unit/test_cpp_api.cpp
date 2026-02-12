#include <gtest/gtest.h>

#include "lvkw/lvkw.hpp"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

class CppApiTest : public ::testing::Test {
 protected:
  std::unique_ptr<lvkw::Context> ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.allocator = TrackingAllocator::get_allocator();
    ci.userdata = &tracker;
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
  ctx->pollEvents(LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event&) {});

  EXPECT_EQ(window.getGeometry().pixelSize.x, 1024);
  EXPECT_EQ(window.getGeometry().pixelSize.y, 768);
}

TEST_F(CppApiTest, WindowAttributes) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Attributes Test";
  wci.attributes.logicalSize = {800, 600};

  lvkw::Window window = ctx->createWindow(wci);
  lvkw_mock_markWindowReady(window.get());
  ctx->pollEvents(LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event&) {});

  window.setTitle("New Title");
  window.setSize({1280, 720});

  EXPECT_EQ(window.getGeometry().pixelSize.x, 1280);
}

TEST_F(CppApiTest, ContextAttributes) {
  ctx->setIdleTimeout(1000);
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
  ev.type = LVKW_EVENT_TYPE_KEY;
  ev.window = window.get();
  ev.key.key = LVKW_KEY_ESCAPE;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;
  lvkw_mock_pushEvent(ctx->get(), &ev);

  bool received_key = false;
  bool received_ready = false;

  ctx->pollEvents(
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
  ev.type = LVKW_EVENT_TYPE_MOUSE_BUTTON;
  ev.window = window.get();
  ev.mouse_button.button = LVKW_MOUSE_BUTTON_LEFT;
  ev.mouse_button.state = LVKW_BUTTON_STATE_PRESSED;
  lvkw_mock_pushEvent(ctx->get(), &ev);

  bool received = false;
  ctx->pollEvents(LVKW_EVENT_TYPE_MOUSE_BUTTON, [&](const LVKW_Event& e) {
    EXPECT_EQ(e.type, LVKW_EVENT_TYPE_MOUSE_BUTTON);
    EXPECT_EQ(e.window, window.get());
    EXPECT_EQ(e.mouse_button.button, LVKW_MOUSE_BUTTON_LEFT);
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
  ev.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;
  ev.window = window.get();
  ev.resized.geometry.logicalSize = {1280, 720};
  lvkw_mock_pushEvent(ctx->get(), &ev);

  bool resized = false;
  auto visitor = lvkw::overloads{
      [&](lvkw::WindowResizedEvent e) {
        EXPECT_EQ(e->geometry.logicalSize.x, 1280);
        resized = true;
      },
      [](auto&&) {}  // catch-all
  };

  ctx->pollEvents(visitor);
  EXPECT_TRUE(resized);
}

TEST_F(CppApiTest, GetMonitorsEmpty) {
  auto monitors = ctx->getMonitors();
  EXPECT_TRUE(monitors.empty());
}

TEST_F(CppApiTest, GetMonitorsWithMock) {
  LVKW_MonitorInfo info = {};
  info.id = 1;
  info.name = "C++ Test Monitor";
  info.physical_size = {600, 340};
  info.current_mode = {{1920, 1080}, 60000};
  info.is_primary = true;
  info.scale = 1.5f;

  lvkw_mock_addMonitor(ctx->get(), &info);

  auto monitors = ctx->getMonitors();
  ASSERT_EQ(monitors.size(), 1);
  EXPECT_EQ(monitors[0].id, 1);
  EXPECT_STREQ(monitors[0].name, "C++ Test Monitor");
  EXPECT_FLOAT_EQ(monitors[0].scale, 1.5f);
}

TEST_F(CppApiTest, GetMonitorModes) {
  LVKW_MonitorInfo info = {};
  info.id = 1;
  info.name = "Mode Monitor";
  lvkw_mock_addMonitor(ctx->get(), &info);

  LVKW_VideoMode mode = {{3840, 2160}, 60000};
  lvkw_mock_addMonitorMode(ctx->get(), 1, mode);

  auto modes = ctx->getMonitorModes(1);
  ASSERT_EQ(modes.size(), 1);
  EXPECT_EQ(modes[0].size.x, 3840);
  EXPECT_EQ(modes[0].refresh_rate_mhz, 60000);
}

TEST_F(CppApiTest, MonitorConnectionEventVisitor) {
  LVKW_MonitorInfo info = {};
  info.id = 7;
  info.name = "Event Monitor";
  info.is_primary = false;

  lvkw_mock_addMonitor(ctx->get(), &info);

  bool got_event = false;
  ctx->pollEvents([&](lvkw::MonitorConnectionEvent e) {
    EXPECT_TRUE(e->connected);
    EXPECT_EQ(e->monitor.id, 7);
    got_event = true;
  });

  EXPECT_TRUE(got_event);
}

TEST_F(CppApiTest, PartialVisitorFlushesUnhandled) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "C++ Test Window";
  wci.attributes.logicalSize = {1024, 768};
  lvkw::Window window = ctx->createWindow(wci);

  // Push two events: one handled by visitor, one NOT
  LVKW_Event ev_key = {};
  ev_key.type = LVKW_EVENT_TYPE_KEY;
  ev_key.window = window.get();
  ev_key.key.key = LVKW_KEY_A;
  lvkw_mock_pushEvent(ctx->get(), &ev_key);

  LVKW_Event ev_motion = {};
  ev_motion.type = LVKW_EVENT_TYPE_MOUSE_MOTION;
  ev_motion.window = window.get();
  ev_motion.mouse_motion.position.x = 42;
  lvkw_mock_pushEvent(ctx->get(), &ev_motion);

  int key_calls = 0;
  int motion_calls = 0;

  // Visitor only handles keys. This infers mask=KEY.
  // The C-level queue will pop KEY (match) and flush MOTION (mismatch).
  ctx->pollEvents([&](lvkw::KeyboardEvent) { key_calls++; });

  EXPECT_EQ(key_calls, 1);
  EXPECT_EQ(motion_calls, 0);

  // Verify motion event is GONE (flushed)
  ctx->pollEvents([&](lvkw::MouseMotionEvent e) {
    motion_calls++;
  });
  EXPECT_EQ(motion_calls, 0);
}
