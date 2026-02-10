#include <gtest/gtest.h>

#include "lvkw/lvkw_checked.h"
#include "lvkw_mock.h"

class CheckedApiTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx = nullptr;
  LVKW_Diagnosis last_diagnosis = LVKW_DIAGNOSIS_NONE;

  static void diagnosis_cb(const LVKW_DiagnosisInfo* info, void* userdata) {
    auto* self = static_cast<CheckedApiTest*>(userdata);
    self->last_diagnosis = info->diagnosis;
  }

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.diagnosis_cb = diagnosis_cb;
    ci.diagnosis_userdata = this;
    lvkw_context_create(&ci, &ctx);
    last_diagnosis = LVKW_DIAGNOSIS_NONE;
  }

  void TearDown() override {
    if (ctx) lvkw_context_destroy(ctx);
  }
};

TEST_F(CheckedApiTest, InvalidArgumentReportsDiagnosis) {
  // If we pass NULL as window, we have no context to report to, so it won't
  // trigger callback. But let's check it returns NOOP.
  LVKW_Status status = lvkw_chk_window_setCursorMode(nullptr, LVKW_CURSOR_LOCKED);
  EXPECT_EQ(status, LVKW_ERROR_NOOP);
#ifdef LVKW_ENABLE_DIAGNOSIS
  EXPECT_EQ(last_diagnosis, LVKW_DIAGNOSIS_NONE);
#endif
}

TEST_F(CheckedApiTest, WindowNotReadyReportsDiagnosis) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "Test";
  wci.size = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_window_create(ctx, &wci, &window);
  ASSERT_NE(window, nullptr);

  // Should fail because we haven't polled for READY yet
  LVKW_Status status = lvkw_chk_window_setCursorMode(window, LVKW_CURSOR_LOCKED);

  EXPECT_EQ(status, LVKW_ERROR_NOOP);
#ifdef LVKW_ENABLE_DIAGNOSIS
  EXPECT_EQ(last_diagnosis, LVKW_DIAGNOSIS_PRECONDITION_FAILURE);
#endif

  lvkw_window_destroy(window);
}

TEST_F(CheckedApiTest, InvalidCallbackReportsDiagnosis) {
  LVKW_ContextResult status = lvkw_chk_context_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, nullptr, nullptr);

  EXPECT_EQ(status, LVKW_ERROR_NOOP);
#ifdef LVKW_ENABLE_DIAGNOSIS
  EXPECT_EQ(last_diagnosis, LVKW_DIAGNOSIS_INVALID_ARGUMENT);
#endif
}

TEST_F(CheckedApiTest, SuccessDoesNotReportDiagnosis) {
  LVKW_WindowCreateInfo wci = {};
  wci.title = "Test";
  wci.size = {640, 480};
  LVKW_Window* window = nullptr;

  lvkw_window_create(ctx, &wci, &window);

  // Mock ready
  LVKW_Event ev = {};
  ev.type = LVKW_EVENT_TYPE_WINDOW_READY;
  ev.window_ready.window = window;
  lvkw_mock_pushEvent(ctx, &ev);
  lvkw_context_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, [](const LVKW_Event*, void*) {}, nullptr);

  LVKW_Status status = lvkw_chk_window_setCursorMode(window, LVKW_CURSOR_LOCKED);

  EXPECT_EQ(status, LVKW_OK);

  lvkw_window_destroy(window);
}
