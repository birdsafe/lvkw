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
      lvkw_destroyContext(ctx);
    }
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }
};

TEST_F(MockBackendTest, WindowCreation) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "Test Window";
  wci.app_id = "test.app";
  wci.size = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_context_createWindow(ctx, &wci, &window), LVKW_SUCCESS);
  ASSERT_NE(window, nullptr);

  // Make window ready
  lvkw_context_pollEvents(ctx, LVKW_EVENT_TYPE_WINDOW_READY, [](const LVKW_Event*, void*) {}, nullptr);

  LVKW_Size size;
  ASSERT_EQ(lvkw_window_getFramebufferSize(window, &size), LVKW_SUCCESS);
  EXPECT_EQ(size.width, 800);
  EXPECT_EQ(size.height, 600);

  lvkw_destroyWindow(window);
}

TEST_F(MockBackendTest, EventPushPoll) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "Test Window";
  wci.size = {800, 600};

  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_context_createWindow(ctx, &wci, &window), LVKW_SUCCESS);

  // Create a mock event
  LVKW_Event ev = {};
  ev.type = LVKW_EVENT_TYPE_KEY;
  ev.window = window;
  ev.key.key = LVKW_KEY_A;
  ev.key.state = LVKW_BUTTON_STATE_PRESSED;

  lvkw_mock_pushEvent(ctx, &ev);

  bool received = false;
  ASSERT_EQ(lvkw_context_pollEvents(
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

  lvkw_destroyWindow(window);
}
