// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include <atomic>
#include <thread>

#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "lvkw_mock.h"

class ValidationTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx = nullptr;
  LVKW_Diagnostic last_diagnostic = LVKW_DIAGNOSTIC_NONE;

  static void diagnostic_cb(const LVKW_DiagnosticInfo* info, void* userdata) {
    auto* self = static_cast<ValidationTest*>(userdata);
    self->last_diagnostic = info->diagnostic;
  }

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.attributes.diagnostic_cb = diagnostic_cb;
    ci.attributes.diagnostic_userdata = this;
    lvkw_context_create(&ci, &ctx);
    last_diagnostic = LVKW_DIAGNOSTIC_NONE;
  }

  void TearDown() override {
    if (ctx) lvkw_context_destroy(ctx);
  }
};

TEST_F(ValidationTest, InvalidArgumentReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_display_updateWindow(nullptr, LVKW_WINDOW_ATTR_CURSOR_MODE, &attrs);
  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(ValidationTest, WindowNotReadyReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logicalSize = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_display_createWindow(ctx, &wci, &window);
  ASSERT_NE(window, nullptr);

  // Should fail because we haven't polled for READY yet
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_CURSOR_MODE, &attrs);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE);

  lvkw_display_destroyWindow(window);
#endif
}

TEST_F(ValidationTest, InvalidCallbackReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Status status = lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, nullptr, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(ValidationTest, ContextAttributesReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Status status = lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_IDLE_TIMEOUT, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(ValidationTest, ClipboardValidation) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logicalSize = {640, 480};
  LVKW_Window* window = nullptr;
  lvkw_display_createWindow(ctx, &wci, &window);
  lvkw_mock_markWindowReady(window);

  // setClipboardText(NULL)
  EXPECT_EQ(lvkw_data_setClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);

  // getClipboardText(NULL)
  EXPECT_EQ(lvkw_data_getClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);

  // setClipboardData(NULL, 1)
  EXPECT_EQ(lvkw_data_setClipboardData(window, nullptr, 1), LVKW_ERROR_INVALID_USAGE);

  // getClipboardData(mime, NULL, size)
  size_t size;
  EXPECT_EQ(lvkw_data_getClipboardData(window, "text/plain", nullptr, &size), LVKW_ERROR_INVALID_USAGE);

  // getClipboardMimeTypes(NULL, NULL)
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, nullptr, nullptr), LVKW_ERROR_INVALID_USAGE);

  // getClipboardMimeTypes(NULL, &count) is valid (count-only query)
  uint32_t mime_count = 123;
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, nullptr, &mime_count), LVKW_SUCCESS);
  EXPECT_EQ(mime_count, 0u);

  // getClipboardMimeTypes(&ptr, &count) should set both outputs.
  const char **mime_types = reinterpret_cast<const char **>(0x1);
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, &mime_types, &mime_count), LVKW_SUCCESS);
  EXPECT_EQ(mime_types, nullptr);
  EXPECT_EQ(mime_count, 0u);

  lvkw_display_destroyWindow(window);
#endif
}

TEST_F(ValidationTest, AbortOnViolation) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  EXPECT_DEATH(lvkw_context_update(nullptr, LVKW_CONTEXT_ATTR_IDLE_TIMEOUT, nullptr), "");
#endif
}

TEST_F(ValidationTest, ThreadAffinityEnforcedForPrimaryThreadApi) {
#ifdef LVKW_VALIDATE_API_CALLS
  LVKW_ContextAttributes attrs = {};
  attrs.idle_timeout_ms = 16;
#ifdef LVKW_RECOVERABLE_API_CALLS
  std::atomic<int> status{LVKW_SUCCESS};
  std::thread t([&]() {
    status.store((int)lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_IDLE_TIMEOUT, &attrs),
                 std::memory_order_relaxed);
  });
  t.join();
  EXPECT_EQ((LVKW_Status)status.load(std::memory_order_relaxed), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE);
#else
  EXPECT_DEATH({
    std::thread t([&]() {
      lvkw_context_update(ctx, LVKW_CONTEXT_ATTR_IDLE_TIMEOUT, &attrs);
    });
    t.join();
  }, "");
#endif
#endif
}
