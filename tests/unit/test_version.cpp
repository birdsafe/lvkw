#include <gtest/gtest.h>

#include "lvkw/lvkw.h"

TEST(VersionTest, MacrosMatch) {
  LVKW_Version version = lvkw_getVersion();

  EXPECT_EQ(version.major, LVKW_VERSION_MAJOR);
  EXPECT_EQ(version.minor, LVKW_VERSION_MINOR);
  EXPECT_EQ(version.patch, LVKW_VERSION_PATCH);
}
