// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include <vector>

#include "lvkw/lvkw.h"
#include "api_constraints.h"

#ifdef LVKW_ENABLE_CONTROLLER
class InputContractTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx = nullptr;
  LVKW_Controller* ctrl = nullptr;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    ASSERT_EQ(lvkw_context_create(&ci, &ctx), LVKW_SUCCESS);

    uint32_t count = 0;
    ASSERT_EQ(lvkw_input_listControllers(ctx, nullptr, &count), LVKW_SUCCESS);
    ASSERT_GT(count, 0u);

    std::vector<LVKW_ControllerRef*> refs(count);
    ASSERT_EQ(lvkw_input_listControllers(ctx, refs.data(), &count), LVKW_SUCCESS);
    ASSERT_EQ(lvkw_input_createController(refs[0], &ctrl), LVKW_SUCCESS);
    ASSERT_NE(ctrl, nullptr);
  }

  void TearDown() override {
    if (ctrl) lvkw_input_destroyController(ctrl);
    if (ctx) lvkw_context_destroy(ctx);
  }
};

TEST_F(InputContractTest, DirectConstraintRejectsNullIntensities) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  EXPECT_EQ(_lvkw_api_constraints_ctrl_setHapticLevels(ctrl, 0, 1, nullptr), LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(InputContractTest, DirectConstraintRejectsOutOfRange) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Scalar levels[] = {0.25f, 0.5f, 0.75f, 1.0f, 0.5f};
  EXPECT_EQ(_lvkw_api_constraints_ctrl_setHapticLevels(ctrl, 0, 5, levels), LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(InputContractTest, DirectConstraintRejectsInvalidIntensity) {
#ifdef LVKW_RECOVERABLE_API_CALLS
  LVKW_Scalar levels[] = {1.5f};
  EXPECT_EQ(_lvkw_api_constraints_ctrl_setHapticLevels(ctrl, 0, 1, levels), LVKW_ERROR_INVALID_USAGE);
#endif
}

TEST_F(InputContractTest, HapticsApiSmoke) {
  LVKW_Scalar levels[] = {0.2f, 0.7f};
  EXPECT_EQ(lvkw_input_setControllerHapticLevels(ctrl, 0, 2, levels), LVKW_SUCCESS);
}
#endif
