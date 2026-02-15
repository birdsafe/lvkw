// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>
#include "lvkw/lvkw.h"

TEST(ABITest, EventSizeThreshold) {
    // The library guarantees that LVKW_Event fits within specific cache-line friendly sizes.
    // 32 bytes for Float config.
    // 48 bytes for Double config.
    size_t threshold = (sizeof(LVKW_Scalar) == 4) ? 32 : 48;
    
    EXPECT_LE(sizeof(LVKW_Event), threshold) 
        << "LVKW_Event size (" << sizeof(LVKW_Event) 
        << ") exceeds guaranteed threshold (" << threshold << ") for " 
        << ((sizeof(LVKW_Scalar) == 4) ? "Float" : "Double") << " config.\n"
        << "This implies one of the event structures in the union has grown too large.";
}
