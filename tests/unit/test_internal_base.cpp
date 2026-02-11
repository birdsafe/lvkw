#include <gtest/gtest.h>

extern "C" {
#include "lvkw_internal.h"
}

TEST(InternalBaseTest, WindowListManagement) {
  LVKW_Context_Base ctx = {};
  LVKW_Window_Base w1 = {};
  LVKW_Window_Base w2 = {};
  LVKW_Window_Base w3 = {};

  // Test Adding
  _lvkw_window_list_add(&ctx, &w1);
  _lvkw_window_list_add(&ctx, &w2);
  _lvkw_window_list_add(&ctx, &w3);

  // List should be LIFO: w3 -> w2 -> w1
  EXPECT_EQ(ctx.prv.window_list, &w3);
  EXPECT_EQ(w3.prv.next, &w2);
  EXPECT_EQ(w2.prv.next, &w1);
  EXPECT_EQ(w1.prv.next, nullptr);

  // Test Removing Middle
  _lvkw_window_list_remove(&ctx, &w2);
  EXPECT_EQ(ctx.prv.window_list, &w3);
  EXPECT_EQ(w3.prv.next, &w1);
  EXPECT_EQ(w1.prv.next, nullptr);

  // Test Removing Head
  _lvkw_window_list_remove(&ctx, &w3);
  EXPECT_EQ(ctx.prv.window_list, &w1);
  EXPECT_EQ(w1.prv.next, nullptr);

  // Test Removing Last
  _lvkw_window_list_remove(&ctx, &w1);
  EXPECT_EQ(ctx.prv.window_list, nullptr);
}

TEST(InternalBaseTest, ContextMarkLostPropagatesToWindows) {
  LVKW_Context_Base ctx = {};
  LVKW_Window_Base w1 = {};
  LVKW_Window_Base w2 = {};
  LVKW_Window_Base w3 = {};

  _lvkw_window_list_add(&ctx, &w1);
  _lvkw_window_list_add(&ctx, &w2);
  _lvkw_window_list_add(&ctx, &w3);

  // Initially healthy
  EXPECT_FALSE(ctx.pub.flags & LVKW_CTX_STATE_LOST);
  EXPECT_FALSE(w1.pub.flags & LVKW_WND_STATE_LOST);
  EXPECT_FALSE(w2.pub.flags & LVKW_WND_STATE_LOST);
  EXPECT_FALSE(w3.pub.flags & LVKW_WND_STATE_LOST);

  // Mark context as lost
  _lvkw_context_mark_lost(&ctx);

  // Everything should be lost
  EXPECT_TRUE(ctx.pub.flags & LVKW_CTX_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WND_STATE_LOST);
  EXPECT_TRUE(w2.pub.flags & LVKW_WND_STATE_LOST);
  EXPECT_TRUE(w3.pub.flags & LVKW_WND_STATE_LOST);
}

TEST(InternalBaseTest, MarkLostIsIdempotent) {
  LVKW_Context_Base ctx = {};
  LVKW_Window_Base w1 = {};
  _lvkw_window_list_add(&ctx, &w1);

  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CTX_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WND_STATE_LOST);

  // Call again, should not crash or change state
  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CTX_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WND_STATE_LOST);
}

TEST(InternalBaseTest, MarkLostEmptyList) {
  LVKW_Context_Base ctx = {};
  EXPECT_FALSE(ctx.pub.flags & LVKW_CTX_STATE_LOST);

  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CTX_STATE_LOST);
}

TEST(InternalBaseTest, ThreadAffinityInit) {
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
  LVKW_ContextCreateInfo info = {};
  LVKW_Context_Base ctx;

  _lvkw_context_init_base(&ctx, &info);

  EXPECT_EQ(_lvkw_get_current_thread_id(), ctx.prv.creator_thread);
#endif
}

#include <thread>

#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
static void test_affinity_violation(LVKW_Context_Base *ctx) {
  std::thread t([ctx]() { LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx); });
  t.join();
}
#endif

TEST(InternalBaseTest, ThreadAffinityViolationDetection) {
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
  LVKW_ContextCreateInfo info = {};
  LVKW_Context_Base ctx;
  _lvkw_context_init_base(&ctx, &info);

  // This should pass on creator thread
  LVKW_CTX_ASSERT_THREAD_AFFINITY(&ctx);

  // This should fail on another thread.
  // Note: Since LVKW_CTX_ASSERT_THREAD_AFFINITY calls abort(), we test it with DeathTests.
  EXPECT_DEATH(test_affinity_violation(&ctx), "");
#endif
}
