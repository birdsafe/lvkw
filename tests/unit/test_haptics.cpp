#include <gtest/gtest.h>
#include "lvkw/lvkw.h"
#include "lvkw/lvkw-ext-controller.h"
#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"

class HapticsTest : public ::testing::Test {
protected:
    LVKW_Context* ctx;

    void SetUp() override {
        LVKW_ContextCreateInfo create_info = {};
        create_info.backend = LVKW_BACKEND_AUTO;
        ASSERT_EQ(lvkw_createContext(&create_info, &ctx), LVKW_SUCCESS);
    }

    void TearDown() override {
        lvkw_ctx_destroy(ctx);
    }
};

TEST_F(HapticsTest, MotorLevelSetting) {
#ifndef LVKW_CONTROLLER_ENABLED
    GTEST_SKIP() << "Controller support not enabled";
#endif

    LVKW_Controller* ctrl;
    // Mock 0 is a valid ID for our mock
    ASSERT_EQ(lvkw_ctrl_create(ctx, 0, &ctrl), LVKW_SUCCESS);
    ASSERT_NE(ctrl, nullptr);
    ASSERT_GT(ctrl->motor_count, 0u);

    LVKW_real_t levels[] = { 0.5f, 0.8f };
    EXPECT_EQ(lvkw_ctrl_setMotorLevels(ctrl, 0, 2, levels), LVKW_SUCCESS);

    // Verify mock state
    LVKW_Controller_Mock* mock_ctrl = (LVKW_Controller_Mock*)ctrl;
    EXPECT_FLOAT_EQ(mock_ctrl->motor_levels[0], 0.5f);
    EXPECT_FLOAT_EQ(mock_ctrl->motor_levels[1], 0.8f);

    levels[0] = 1.0f;
    EXPECT_EQ(lvkw_ctrl_setMotorLevels(ctrl, 0, 1, levels), LVKW_SUCCESS);
    EXPECT_FLOAT_EQ(mock_ctrl->motor_levels[0], 1.0f);

    lvkw_ctrl_destroy(ctrl);
}

TEST_F(HapticsTest, Validation) {
#ifndef LVKW_CONTROLLER_ENABLED
    GTEST_SKIP() << "Controller support not enabled";
#endif

    LVKW_Controller* ctrl;
    ASSERT_EQ(lvkw_ctrl_create(ctx, 0, &ctrl), LVKW_SUCCESS);

    LVKW_real_t levels[] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    
    // Out of bounds
    // Note: In debug builds this might abort if LVKW_RECOVERABLE_API_CALLS is not set.
    // Assuming we want to test the status return in this environment.
#ifdef LVKW_RECOVERABLE_API_CALLS
    EXPECT_EQ(lvkw_ctrl_setMotorLevels(ctrl, 0, 5, levels), LVKW_ERROR_INVALID_USAGE);
    
    // Invalid intensity
    levels[0] = 1.5f;
    EXPECT_EQ(lvkw_ctrl_setMotorLevels(ctrl, 0, 1, levels), LVKW_ERROR_INVALID_USAGE);
    
    levels[0] = -0.1f;
    EXPECT_EQ(lvkw_ctrl_setMotorLevels(ctrl, 0, 1, levels), LVKW_ERROR_INVALID_USAGE);
#endif

    lvkw_ctrl_destroy(ctrl);
}
