#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "lvkw_mock.h"
#include "test_helpers.hpp"

extern "C" {
#include "lvkw_string_cache.h"
#include "lvkw_types_internal.h"
}

class StringCacheTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.allocator = TrackingAllocator::get_allocator();
    ci.userdata = &tracker;
    ASSERT_EQ(lvkw_createContext(&ci, &ctx), LVKW_SUCCESS);
  }

  void TearDown() override {
    if (ctx) {
      lvkw_ctx_destroy(ctx);
    }
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }

  LVKW_StringCache& cache() { return ((LVKW_Context_Base*)ctx)->prv.string_cache; }
  LVKW_Context_Base* base() { return (LVKW_Context_Base*)ctx; }
};

TEST_F(StringCacheTest, InitiallyEmpty) {
  EXPECT_EQ(cache().count, 0);
}

TEST_F(StringCacheTest, InternReturnsValidCopy) {
  const char* result = _lvkw_string_cache_intern(&cache(), base(), "hello");
  ASSERT_NE(result, nullptr);
  EXPECT_STREQ(result, "hello");
  EXPECT_EQ(cache().count, 1);
}

TEST_F(StringCacheTest, InternReturnsSamePointerForDuplicate) {
  const char* first = _lvkw_string_cache_intern(&cache(), base(), "duplicate");
  const char* second = _lvkw_string_cache_intern(&cache(), base(), "duplicate");

  EXPECT_EQ(first, second);
  EXPECT_EQ(cache().count, 1);
}

TEST_F(StringCacheTest, InternDistinguishesDifferentStrings) {
  const char* a = _lvkw_string_cache_intern(&cache(), base(), "alpha");
  const char* b = _lvkw_string_cache_intern(&cache(), base(), "beta");

  EXPECT_NE(a, b);
  EXPECT_STREQ(a, "alpha");
  EXPECT_STREQ(b, "beta");
  EXPECT_EQ(cache().count, 2);
}

TEST_F(StringCacheTest, InternedStringIsIndependentOfSource) {
  char buf[32];
  snprintf(buf, sizeof(buf), "temp");
  const char* interned = _lvkw_string_cache_intern(&cache(), base(), buf);

  // Mutate the source buffer
  buf[0] = 'X';

  EXPECT_STREQ(interned, "temp");
}

TEST_F(StringCacheTest, InternNullReturnsNull) {
  const char* result = _lvkw_string_cache_intern(&cache(), base(), nullptr);
  EXPECT_EQ(result, nullptr);
  EXPECT_EQ(cache().count, 0);
}

TEST_F(StringCacheTest, InternEmptyString) {
  const char* result = _lvkw_string_cache_intern(&cache(), base(), "");
  ASSERT_NE(result, nullptr);
  EXPECT_STREQ(result, "");
  EXPECT_EQ(cache().count, 1);

  // Interning empty string again returns same pointer
  const char* again = _lvkw_string_cache_intern(&cache(), base(), "");
  EXPECT_EQ(result, again);
  EXPECT_EQ(cache().count, 1);
}

TEST_F(StringCacheTest, CacheFullReturnsNull) {
  // Fill the cache to capacity
  for (uint32_t i = 0; i < LVKW_STRING_CACHE_MAX_ENTRIES; i++) {
    char name[32];
    snprintf(name, sizeof(name), "monitor_%u", i);
    const char* result = _lvkw_string_cache_intern(&cache(), base(), name);
    ASSERT_NE(result, nullptr) << "Failed to intern entry " << i;
  }

  EXPECT_EQ(cache().count, LVKW_STRING_CACHE_MAX_ENTRIES);

  // Next intern should fail
  const char* overflow = _lvkw_string_cache_intern(&cache(), base(), "overflow");
  EXPECT_EQ(overflow, nullptr);
  EXPECT_EQ(cache().count, LVKW_STRING_CACHE_MAX_ENTRIES);
}

TEST_F(StringCacheTest, ManyUniqueStrings) {
  const char* ptrs[16];
  for (int i = 0; i < 16; i++) {
    char name[32];
    snprintf(name, sizeof(name), "display-%d", i);
    ptrs[i] = _lvkw_string_cache_intern(&cache(), base(), name);
    ASSERT_NE(ptrs[i], nullptr);
  }

  EXPECT_EQ(cache().count, 16);

  // All pointers should be distinct
  for (int i = 0; i < 16; i++) {
    for (int j = i + 1; j < 16; j++) {
      EXPECT_NE(ptrs[i], ptrs[j]);
    }
  }

  // Re-interning any of them should return the same pointer
  for (int i = 0; i < 16; i++) {
    char name[32];
    snprintf(name, sizeof(name), "display-%d", i);
    EXPECT_EQ(_lvkw_string_cache_intern(&cache(), base(), name), ptrs[i]);
  }

  EXPECT_EQ(cache().count, 16);
}

TEST_F(StringCacheTest, DestroyFreesAllStrings) {
  for (int i = 0; i < 5; i++) {
    char name[32];
    snprintf(name, sizeof(name), "str_%d", i);
    _lvkw_string_cache_intern(&cache(), base(), name);
  }

  size_t allocs_before = tracker.active_allocations();
  _lvkw_string_cache_destroy(&cache(), base());
  size_t allocs_after = tracker.active_allocations();

  // 5 string allocations should have been freed
  EXPECT_EQ(allocs_before - allocs_after, 5);
  EXPECT_EQ(cache().count, 0);

  // Re-init so TearDown's destroy doesn't double-free
  _lvkw_string_cache_init(&cache());
}
