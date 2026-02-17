// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_mock.h"

class DataContractTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx = nullptr;
  LVKW_Diagnostic last_diagnostic = LVKW_DIAGNOSTIC_NONE;

  static void diagnostic_cb(const LVKW_DiagnosticInfo* info, void* userdata) {
    auto* self = static_cast<DataContractTest*>(userdata);
    self->last_diagnostic = info->diagnostic;
  }

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.attributes.diagnostic_cb = diagnostic_cb;
    ci.attributes.diagnostic_userdata = this;
    ASSERT_EQ(lvkw_context_create(&ci, &ctx), LVKW_SUCCESS);
    last_diagnostic = LVKW_DIAGNOSTIC_NONE;
  }

  void TearDown() override {
    if (ctx) lvkw_context_destroy(ctx);
  }
};

TEST_F(DataContractTest, ClipboardValidation) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_WindowCreateInfo wci = {};
  wci.attributes.title = "Test";
  wci.attributes.logical_size = {640, 480};
  LVKW_Window* window = nullptr;
  ASSERT_EQ(lvkw_display_createWindow(ctx, &wci, &window), LVKW_SUCCESS);
  lvkw_mock_markWindowReady(window);

  EXPECT_EQ(lvkw_data_setClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(last_diagnostic, LVKW_DIAGNOSTIC_INVALID_ARGUMENT);

  EXPECT_EQ(lvkw_data_getClipboardText(window, nullptr), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(lvkw_data_setClipboardData(window, nullptr, 1), LVKW_ERROR_INVALID_USAGE);

  size_t size = 0;
  EXPECT_EQ(lvkw_data_getClipboardData(window, "text/plain", nullptr, &size), LVKW_ERROR_INVALID_USAGE);
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, nullptr, nullptr), LVKW_ERROR_INVALID_USAGE);

  uint32_t mime_count = 123;
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, nullptr, &mime_count), LVKW_SUCCESS);
  EXPECT_EQ(mime_count, 0u);

  const char** mime_types = reinterpret_cast<const char**>(0x1);
  EXPECT_EQ(lvkw_data_getClipboardMimeTypes(window, &mime_types, &mime_count), LVKW_SUCCESS);
  EXPECT_EQ(mime_types, nullptr);
  EXPECT_EQ(mime_count, 0u);

  lvkw_display_destroyWindow(window);
#endif
}
