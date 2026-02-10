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
    wci.title = "Scoped Window";
    wci.app_id = "scoped.app";
    wci.size = {640, 480};
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
  wci.title = "C++ Test Window";
  wci.size = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);
  EXPECT_NE(window.get(), nullptr);

  // Make window ready
  ctx->pollEvents(LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event&) {});

  EXPECT_EQ(window.getFramebufferSize().width, 1024);
  EXPECT_EQ(window.getFramebufferSize().height, 768);
}

TEST_F(CppApiTest, EventVisitor) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "C++ Test Window";
  wci.size = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

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
  wci.title = "C++ Test Window";
  wci.size = {1024, 768};

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
  wci.title = "C++ Test Window";
  wci.size = {1024, 768};

  lvkw::Window window = ctx->createWindow(wci);

  LVKW_Event ev = {};
  ev.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;
  ev.window = window.get();
  ev.resized.size = {1280, 720};
  lvkw_mock_pushEvent(ctx->get(), &ev);

  bool resized = false;
  auto visitor = lvkw::overloads{
      [&](lvkw::WindowResizedEvent e) {
        EXPECT_EQ(e->size.width, 1280);
        resized = true;
      },
      [](auto&&) {}  // catch-all
  };

  ctx->pollEvents(visitor);
  EXPECT_TRUE(resized);
}

TEST_F(CppApiTest, PartialVisitor) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "C++ Test Window";
  wci.size = {1024, 768};
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
  ev_motion.mouse_motion.x = 42;
  lvkw_mock_pushEvent(ctx->get(), &ev_motion);

  int key_calls = 0;
  int motion_calls = 0;

  // Visitor only handles keys
  ctx->pollEvents([&](lvkw::KeyboardEvent) { key_calls++; });

  EXPECT_EQ(key_calls, 1);
  EXPECT_EQ(motion_calls, 0);

  // Verify motion event is still in the queue by polling it specifically
  ctx->pollEvents([&](lvkw::MouseMotionEvent e) {
    EXPECT_EQ(e->x, 42);
    motion_calls++;
  });
  EXPECT_EQ(motion_calls, 1);
}
