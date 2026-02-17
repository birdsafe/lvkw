// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <gtest/gtest.h>

#include "lvkw/lvkw.h"
#include "test_helpers.hpp"

extern "C" {
#include "transient_pool.h"
#include "types_internal.h"
}

class TransientPoolTest : public ::testing::Test {
 protected:
  LVKW_Context* ctx;
  TrackingAllocator tracker;

  void SetUp() override {
    LVKW_ContextCreateInfo ci = {};
    ci.allocator = TrackingAllocator::get_allocator();
    ci.allocator.userdata = &tracker;
    ASSERT_EQ(lvkw_context_create(&ci, &ctx), LVKW_SUCCESS);
  }

  void TearDown() override {
    if (ctx) {
      lvkw_context_destroy(ctx);
    }
    EXPECT_FALSE(tracker.has_leaks()) << "Leaks detected: " << tracker.active_allocations() << " allocations remaining";
  }

  LVKW_Context_Base* base() { return (LVKW_Context_Base*)ctx; }
};

TEST_F(TransientPoolTest, InitiallyEmpty) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);
  
  EXPECT_EQ(pool.buckets, nullptr);
  EXPECT_EQ(pool.direct_allocs, nullptr);
  
  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, SmallAllocationsUseBuckets) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  void* p1 = _lvkw_transient_pool_alloc(&pool, base(), 100);
  ASSERT_NE(p1, nullptr);
  EXPECT_NE(pool.buckets, nullptr);
  EXPECT_EQ(pool.direct_allocs, nullptr);

  void* p2 = _lvkw_transient_pool_alloc(&pool, base(), 100);
  ASSERT_NE(p2, nullptr);
  
  /* Both should be in the same bucket */
  EXPECT_EQ(pool.buckets, pool.current_bucket);
  EXPECT_LT((uint8_t*)p1, (uint8_t*)p2);

  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, LargeAllocationsAreDirect) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  void* p1 = _lvkw_transient_pool_alloc(&pool, base(), 2048);
  ASSERT_NE(p1, nullptr);
  EXPECT_NE(pool.direct_allocs, nullptr);
  EXPECT_EQ(pool.buckets, nullptr);

  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, PointerStabilityAcrossBucketFill) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  /* Allocate something small */
  const char* str = "stable";
  char* p1 = (char*)_lvkw_transient_pool_alloc(&pool, base(), strlen(str) + 1);
  strcpy(p1, str);

  /* Fill the first bucket (4KB) */
  for (int i = 0; i < 100; i++) {
    _lvkw_transient_pool_alloc(&pool, base(), 50);
  }

  /* p1 should still be valid and contain the same string */
  EXPECT_STREQ(p1, "stable");
  EXPECT_NE(pool.buckets, pool.current_bucket); // Should have moved to a new bucket

  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, ClearResetsBucketsButKeepsThem) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  void* p1 = _lvkw_transient_pool_alloc(&pool, base(), 100);
  ASSERT_NE(pool.buckets, nullptr);
  
  _lvkw_transient_pool_clear(&pool, base());
  EXPECT_NE(pool.buckets, nullptr);
  EXPECT_EQ(pool.buckets->used, 0);

  /* New allocation should reuse the first bucket */
  void* p2 = _lvkw_transient_pool_alloc(&pool, base(), 100);
  EXPECT_EQ(p1, p2);

  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, ClearFreesDirectAllocations) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  size_t allocs_before = tracker.active_allocations();
  
  _lvkw_transient_pool_alloc(&pool, base(), 2048);
  EXPECT_GT(tracker.active_allocations(), allocs_before);

  _lvkw_transient_pool_clear(&pool, base());
  
  /* Direct allocation + Node should be gone. 
     Note: Context itself and its internal state remain. */
  EXPECT_EQ(pool.direct_allocs, nullptr);

  _lvkw_transient_pool_destroy(&pool, base());
}

TEST_F(TransientPoolTest, SizedInterning) {
  LVKW_TransientPool pool;
  _lvkw_transient_pool_init(&pool);

  const char* source = "hello world";
  const char* interned = _lvkw_transient_pool_intern_sized(&pool, base(), source, 5);
  
  ASSERT_NE(interned, nullptr);
  EXPECT_STREQ(interned, "hello");
  EXPECT_EQ(strlen(interned), 5);

  _lvkw_transient_pool_destroy(&pool, base());
}
