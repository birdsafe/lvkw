// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_transient_pool.h"
#include "lvkw_mem_internal.h"

#define LVKW_TRANSIENT_BUCKET_SIZE 4096
#define LVKW_TRANSIENT_DIRECT_THRESHOLD 1024

void _lvkw_transient_pool_init(LVKW_TransientPool *pool) {
  memset(pool, 0, sizeof(*pool));
}

void _lvkw_transient_pool_destroy(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base) {
  _lvkw_transient_pool_clear(pool, ctx_base);

  LVKW_TransientBucket *bucket = pool->buckets;
  while (bucket) {
    LVKW_TransientBucket *next = bucket->next;
    lvkw_context_free(ctx_base, bucket);
    bucket = next;
  }
  
  memset(pool, 0, sizeof(*pool));
}

void *_lvkw_transient_pool_alloc(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, size_t size) {
  size_t aligned_size = (size + 7) & ~(size_t)7;

  /* Path A: Direct allocation for large payloads */
  if (aligned_size > LVKW_TRANSIENT_DIRECT_THRESHOLD) {
    LVKW_DirectAlloc *node = (LVKW_DirectAlloc *)lvkw_context_alloc(ctx_base, sizeof(LVKW_DirectAlloc) + aligned_size);
    if (!node) return NULL;

    node->next = pool->direct_allocs;
    pool->direct_allocs = node;
    return node->data;
  }

  /* Path B: Bucket allocation for small payloads */
  if (!pool->current_bucket || (pool->current_bucket->used + (uint32_t)aligned_size > pool->current_bucket->capacity)) {
    /* Reuse next bucket if it exists */
    if (pool->current_bucket && pool->current_bucket->next) {
      pool->current_bucket = pool->current_bucket->next;
      pool->current_bucket->used = 0;
    } 
    else {
      /* Allocate new bucket */
      LVKW_TransientBucket *new_bucket = (LVKW_TransientBucket *)lvkw_context_alloc(
          ctx_base, sizeof(LVKW_TransientBucket) + LVKW_TRANSIENT_BUCKET_SIZE);
      if (!new_bucket) return NULL;

      new_bucket->next = NULL;
      new_bucket->capacity = LVKW_TRANSIENT_BUCKET_SIZE;
      new_bucket->used = 0;

      if (!pool->buckets) {
        pool->buckets = new_bucket;
      }
      if (pool->current_bucket) {
        pool->current_bucket->next = new_bucket;
      }
      pool->current_bucket = new_bucket;
    }
  }

  void *ptr = pool->current_bucket->data + pool->current_bucket->used;
  pool->current_bucket->used += (uint32_t)aligned_size;
  return ptr;
}

const char *_lvkw_transient_pool_intern(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, const char *str) {
  if (!str) return NULL;
  return _lvkw_transient_pool_intern_sized(pool, ctx_base, str, strlen(str));
}

const char *_lvkw_transient_pool_intern_sized(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, const char *str, size_t len) {
  if (!str) return NULL;
  char *copy = (char *)_lvkw_transient_pool_alloc(pool, ctx_base, len + 1);
  if (copy) {
    memcpy(copy, str, len);
    copy[len] = '\0';
  }
  return copy;
}

void _lvkw_transient_pool_clear(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base) {
  /* 1. Free all direct allocations */
  LVKW_DirectAlloc *direct = pool->direct_allocs;
  while (direct) {
    LVKW_DirectAlloc *next = direct->next;
    lvkw_context_free(ctx_base, direct);
    direct = next;
  }
  pool->direct_allocs = NULL;

  /* 2. Reset bucket chain */
  LVKW_TransientBucket *bucket = pool->buckets;
  while (bucket) {
    bucket->used = 0;
    bucket = bucket->next;
  }
  pool->current_bucket = pool->buckets;
}
