// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include "lvkw/lvkw.h"
#include "test_helpers.hpp"

class CursorTest : public ::testing::Test {
protected:
    LVKW_Context* ctx = nullptr;

    void SetUp() override {
        LVKW_ContextCreateInfo create_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
        ASSERT_EQ(lvkw_context_create(&create_info, &ctx), LVKW_SUCCESS);
    }

    void TearDown() override {
        if (ctx) {
            lvkw_context_destroy(ctx);
        }
    }
};

TEST_F(CursorTest, GetStandardCursor) {
    LVKW_Cursor* cursor = nullptr;
    ASSERT_EQ(lvkw_display_getStandardCursor(ctx, LVKW_CURSOR_SHAPE_HAND, &cursor), LVKW_SUCCESS);
    ASSERT_NE(cursor, nullptr);
    EXPECT_TRUE(cursor->flags & LVKW_CURSOR_FLAG_SYSTEM);
}

TEST_F(CursorTest, CreateCustomCursor) {
    uint32_t pixels[16 * 16] = {0xFFFFFFFF};
    LVKW_CursorCreateInfo create_info = {};
    create_info.size = {16, 16};
    create_info.hot_spot = {8, 8};
    create_info.pixels = pixels;

    LVKW_Cursor* cursor = nullptr;
    ASSERT_EQ(lvkw_display_createCursor(ctx, &create_info, &cursor), LVKW_SUCCESS);
    ASSERT_NE(cursor, nullptr);
    EXPECT_FALSE(cursor->flags & LVKW_CURSOR_FLAG_SYSTEM);

    EXPECT_EQ(lvkw_display_destroyCursor(cursor), LVKW_SUCCESS);
}

TEST_F(CursorTest, CreateCursorInvalidArgs) {
    LVKW_Cursor* cursor = nullptr;
    LVKW_CursorCreateInfo create_info = {};
    create_info.size = {0, 16}; // Invalid size
    create_info.pixels = (uint32_t*)0x123;

    // This should trigger an abort or return error depending on validation settings.
    // In tests, LVKW_RECOVERABLE_API_CALLS might be ON.
#ifdef LVKW_RECOVERABLE_API_CALLS
    EXPECT_EQ(lvkw_display_createCursor(ctx, &create_info, &cursor), LVKW_ERROR_INVALID_USAGE);
#endif
}
