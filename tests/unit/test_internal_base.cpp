// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

extern "C" {
#include "internal.h"
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
  EXPECT_FALSE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);
  EXPECT_FALSE(w1.pub.flags & LVKW_WINDOW_STATE_LOST);
  EXPECT_FALSE(w2.pub.flags & LVKW_WINDOW_STATE_LOST);
  EXPECT_FALSE(w3.pub.flags & LVKW_WINDOW_STATE_LOST);

  // Mark context as lost
  _lvkw_context_mark_lost(&ctx);

  // Everything should be lost
  EXPECT_TRUE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WINDOW_STATE_LOST);
  EXPECT_TRUE(w2.pub.flags & LVKW_WINDOW_STATE_LOST);
  EXPECT_TRUE(w3.pub.flags & LVKW_WINDOW_STATE_LOST);
}

TEST(InternalBaseTest, MarkLostIsIdempotent) {
  LVKW_Context_Base ctx = {};
  LVKW_Window_Base w1 = {};
  _lvkw_window_list_add(&ctx, &w1);

  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WINDOW_STATE_LOST);

  // Call again, should not crash or change state
  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);
  EXPECT_TRUE(w1.pub.flags & LVKW_WINDOW_STATE_LOST);
}

TEST(InternalBaseTest, MarkLostEmptyList) {
  LVKW_Context_Base ctx = {};
  EXPECT_FALSE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);

  _lvkw_context_mark_lost(&ctx);
  EXPECT_TRUE(ctx.pub.flags & LVKW_CONTEXT_STATE_LOST);
}

TEST(InternalBaseTest, ThreadAffinityInit) {
#if LVKW_API_VALIDATION > 0
  LVKW_ContextCreateInfo info = {};
  LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
  info.tuning = &tuning;
  LVKW_Context_Base ctx;

  ASSERT_EQ(_lvkw_context_init_base(&ctx, &info), LVKW_SUCCESS);

  EXPECT_EQ(_lvkw_get_current_thread_id(), ctx.prv.creator_thread);
  _lvkw_context_cleanup_base(&ctx);
#endif
}

TEST(InternalBaseTest, VkLoaderInit) {
  LVKW_ContextCreateInfo info = {};
  LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
  LVKW_VkGetInstanceProcAddrFunc dummy_loader = (LVKW_VkGetInstanceProcAddrFunc)(uintptr_t)0x1234;
  tuning.vk_loader = dummy_loader;
  info.tuning = &tuning;

  LVKW_Context_Base ctx;
  ASSERT_EQ(_lvkw_context_init_base(&ctx, &info), LVKW_SUCCESS);

  EXPECT_EQ(ctx.prv.vk_loader, dummy_loader);
  _lvkw_context_cleanup_base(&ctx);
}

TEST(InternalBaseTest, WaylandDndPostDropTimeoutDefault) {
  LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
  EXPECT_EQ(tuning.wayland.dnd_post_drop_timeout_ms, 1000u);
}

TEST(InternalBaseTest, WaylandDndPostDropTimeoutCustomAndZero) {
  LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
  tuning.wayland.dnd_post_drop_timeout_ms = 0u;
  EXPECT_EQ(tuning.wayland.dnd_post_drop_timeout_ms, 0u);

  tuning.wayland.dnd_post_drop_timeout_ms = 2500u;
  EXPECT_EQ(tuning.wayland.dnd_post_drop_timeout_ms, 2500u);
}

TEST(InternalBaseTest, WaylandClientSideConstraintsDefaultAndCustom) {
  LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
  EXPECT_TRUE(tuning.wayland.enforce_client_side_constraints);

  tuning.wayland.enforce_client_side_constraints = false;
  EXPECT_FALSE(tuning.wayland.enforce_client_side_constraints);
}
