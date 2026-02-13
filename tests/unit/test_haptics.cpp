// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include "lvkw/lvkw.h"
#include "lvkw/lvkw-ext-controller.h"
#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
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

TEST_F(HapticsTest, HapticLevelSetting) {
#ifndef LVKW_ENABLE_CONTROLLER
    GTEST_SKIP() << "Controller support not enabled";
#endif

    LVKW_Controller* ctrl;
    // Mock 0 is a valid ID for our mock
    ASSERT_EQ(lvkw_ctrl_create(ctx, 0, &ctrl), LVKW_SUCCESS);
    ASSERT_NE(ctrl, nullptr);
    ASSERT_EQ(ctrl->haptic_count, (uint32_t)LVKW_CTRL_HAPTIC_STANDARD_COUNT);
    ASSERT_NE(ctrl->haptic_channels, nullptr);

    EXPECT_STREQ(ctrl->haptic_channels[LVKW_CTRL_HAPTIC_LOW_FREQ].name, "Mock Low Frequency");
    EXPECT_STREQ(ctrl->haptic_channels[LVKW_CTRL_HAPTIC_RIGHT_TRIGGER].name, "Mock Right Trigger");

    LVKW_real_t levels[] = { 0.5f, 0.8f };
    EXPECT_EQ(lvkw_ctrl_setHapticLevels(ctrl, 0, 2, levels), LVKW_SUCCESS);

    // Verify mock state
    LVKW_Controller_Mock* mock_ctrl = (LVKW_Controller_Mock*)ctrl;
    EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[0], 0.5f);
    EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[1], 0.8f);

    levels[0] = 1.0f;
    EXPECT_EQ(lvkw_ctrl_setHapticLevels(ctrl, 0, 1, levels), LVKW_SUCCESS);
    EXPECT_FLOAT_EQ(mock_ctrl->haptic_levels[0], 1.0f);

    lvkw_ctrl_destroy(ctrl);
}

TEST_F(HapticsTest, ControllerMetadata) {
#ifndef LVKW_ENABLE_CONTROLLER
    GTEST_SKIP() << "Controller support not enabled";
#endif

    LVKW_Controller* ctrl;
    ASSERT_EQ(lvkw_ctrl_create(ctx, 0, &ctrl), LVKW_SUCCESS);

    ASSERT_NE(ctrl->analog_channels, nullptr);
    EXPECT_STREQ(ctrl->analog_channels[LVKW_CTRL_ANALOG_LEFT_X].name, "Mock Left Stick X");

    ASSERT_NE(ctrl->button_channels, nullptr);
    EXPECT_STREQ(ctrl->button_channels[LVKW_CTRL_BUTTON_SOUTH].name, "South");

    ASSERT_NE(ctrl->haptic_channels, nullptr);
    EXPECT_STREQ(ctrl->haptic_channels[LVKW_CTRL_HAPTIC_LOW_FREQ].name, "Mock Low Frequency");

    lvkw_ctrl_destroy(ctrl);
}

TEST_F(HapticsTest, Validation) {
#ifndef LVKW_ENABLE_CONTROLLER
    GTEST_SKIP() << "Controller support not enabled";
#endif

    LVKW_Controller* ctrl;
    ASSERT_EQ(lvkw_ctrl_create(ctx, 0, &ctrl), LVKW_SUCCESS);

    LVKW_real_t levels[] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

    // Out of bounds
#ifdef LVKW_RECOVERABLE_API_CALLS
    EXPECT_EQ(lvkw_ctrl_setHapticLevels(ctrl, 0, 5, levels), LVKW_ERROR_INVALID_USAGE);

    // Invalid intensity
    levels[0] = 1.5f;
    EXPECT_EQ(lvkw_ctrl_setHapticLevels(ctrl, 0, 1, levels), LVKW_ERROR_INVALID_USAGE);

    levels[0] = -0.1f;
    EXPECT_EQ(lvkw_ctrl_setHapticLevels(ctrl, 0, 1, levels), LVKW_ERROR_INVALID_USAGE);
#endif

    lvkw_ctrl_destroy(ctrl);
}
#endif