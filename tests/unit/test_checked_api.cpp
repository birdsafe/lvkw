#include <gtest/gtest.h>

#include "lvkw/lvkw_checked.h"
#include "lvkw_mock.h"

class CheckedApiTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx = nullptr;
  LVKW_Diagnostic last_diagnostic = LVKW_DIAGNOSTIC_NONE;

  static void diagnostic_cb(const LVKW_DiagnosticInfo* info, void* userdata) {
    auto* self = static_cast<CheckedApiTest*>(userdata);
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

TEST_F(CheckedApiTest, InvalidArgumentReportsDiagnostic) {
  // If we pass NULL as window, we have no context to report to, so it won't
  // trigger callback. But let's check it returns NOOP.
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_chk_wnd_update(nullptr, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
  EXPECT_EQ(status, LVKW_ERROR);
#ifdef LVKW_ENABLE_DIAGNOSTICS
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_NONE);
#endif
}

TEST_F(CheckedApiTest, WindowNotReadyReportsDiagnostic) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logicalSize = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_ctx_createWindow(ctx, &wci, &window);
  ASSERT_NE(window, nullptr);

  // Should fail because we haven't polled for READY yet
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_chk_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);

  EXPECT_EQ(status, LVKW_ERROR);
#ifdef LVKW_ENABLE_DIAGNOSTICS
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE);
#endif

  lvkw_wnd_destroy(window);
}

TEST_F(CheckedApiTest, InvalidCallbackReportsDiagnostic) {
  LVKW_Status status = lvkw_chk_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, nullptr, nullptr);

  EXPECT_EQ(status, LVKW_ERROR);
#ifdef LVKW_ENABLE_DIAGNOSTICS
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(CheckedApiTest, ContextAttributesReportsDiagnostic) {
  LVKW_Status status = lvkw_chk_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, nullptr);

  EXPECT_EQ(status, LVKW_ERROR);
#ifdef LVKW_ENABLE_DIAGNOSTICS
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(CheckedApiTest, SuccessDoesNotReportDiagnostic) {
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logicalSize = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_ctx_createWindow(ctx, &wci, &window);

  // Mock ready
  LVKW_Event ev = {};
  ev.type = LVKW_EVENT_TYPE_WINDOW_READY;
  ev.window = window;
  lvkw_mock_pushEvent(ctx, &ev);
  lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, [](const LVKW_Event*, void*) {}, nullptr);

  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_mode = LVKW_CURSOR_LOCKED;
  LVKW_Status status = lvkw_chk_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);

  EXPECT_EQ(status, LVKW_SUCCESS);

  lvkw_wnd_destroy(window);
}