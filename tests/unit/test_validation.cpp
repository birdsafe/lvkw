// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
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
    lvkw_createContext(&ci, &ctx);
    last_diagnostic = LVKW_DIAGNOSTIC_NONE;
  }

  void TearDown() override {
    if (ctx) lvkw_ctx_destroy(ctx);
  }
};

TEST_F(ValidationTest, InvalidArgumentReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_wnd_update(nullptr, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(ValidationTest, WindowNotReadyReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logicalSize = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_ctx_createWindow(ctx, &wci, &window);
  ASSERT_NE(window, nullptr);

  // Should fail because we haven't polled for READY yet
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE);

  lvkw_wnd_destroy(window);
#endif
}

TEST_F(ValidationTest, InvalidCallbackReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Status status = lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, nullptr, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(ValidationTest, ContextAttributesReturnsUsageError) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Status status = lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, nullptr);

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
  lvkw_ctx_createWindow(ctx, &wci, &window);
  lvkw_mock_markWindowReady(window);

  // setClipboardText(NULL)
  EXPECT_EQ(lvkw_wnd_setClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);

  // getClipboardText(NULL)
  EXPECT_EQ(lvkw_wnd_getClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);

  // setClipboardData(NULL, 1)
  EXPECT_EQ(lvkw_wnd_setClipboardData(window, nullptr, 1), LVKW_ERROR_INVALID_USAGE);

  // getClipboardData(mime, NULL, size)
  size_t size;
  EXPECT_EQ(lvkw_wnd_getClipboardData(window, "text/plain", nullptr, &size), LVKW_ERROR_INVALID_USAGE);

  // getClipboardMimeTypes(NULL, NULL)
  EXPECT_EQ(lvkw_wnd_getClipboardMimeTypes(window, nullptr, nullptr), LVKW_ERROR_INVALID_USAGE);

  lvkw_wnd_destroy(window);
#endif
}

TEST_F(ValidationTest, AbortOnViolation) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  EXPECT_DEATH(lvkw_ctx_update(nullptr, LVKW_CTX_ATTR_IDLE_TIMEOUT, nullptr), "");
#endif
}
