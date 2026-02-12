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
#if LVKW_API_VALIDATION == LVKW_API_VALIDATION_RECOVER
  LVKW_WindowAttributes attrs = {0};
  LVKW_Status status = lvkw_wnd_update(nullptr, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(ValidationTest, WindowNotReadyReturnsUsageError) {
#if LVKW_API_VALIDATION == LVKW_API_VALIDATION_RECOVER
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
#if LVKW_API_VALIDATION == LVKW_API_VALIDATION_RECOVER
  LVKW_Status status = lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, nullptr, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(ValidationTest, ContextAttributesReturnsUsageError) {
#if LVKW_API_VALIDATION == LVKW_API_VALIDATION_RECOVER
  LVKW_Status status = lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);
#endif
}

TEST_F(ValidationTest, AbortOnViolation) {
#if LVKW_API_VALIDATION == LVKW_API_VALIDATION_RECOVER
  EXPECT_DEATH(lvkw_ctx_update(nullptr, LVKW_CTX_ATTR_IDLE_TIMEOUT, nullptr), "");
#endif
}
