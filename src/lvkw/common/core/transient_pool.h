// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TRANSIENT_POOL_H_INCLUDED
#define LVKW_TRANSIENT_POOL_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration */
typedef struct LVKW_Context_Base LVKW_Context_Base;

#include <stdalign.h>

/**
 * @brief Individual bucket for small transient allocations.
 * Buckets are reused across pool rotations.
 */
typedef struct LVKW_TransientBucket {
  struct LVKW_TransientBucket *next;
  uint32_t capacity;
  uint32_t used;
  alignas(8) uint8_t data[];
} LVKW_TransientBucket;

/**
 * @brief Node for large allocations that bypass the buckets.
 * These are freed immediately upon pool rotation.
 */
typedef struct LVKW_DirectAlloc {
  struct LVKW_DirectAlloc *next;
  alignas(8) uint8_t data[];
} LVKW_DirectAlloc;

/**
 * @brief A standalone transient memory pool.
 * Provides absolute pointer stability until the next clear/rotation.
 */
typedef struct LVKW_TransientPool {
  LVKW_TransientBucket *buckets;
  LVKW_TransientBucket *current_bucket;
  LVKW_DirectAlloc *direct_allocs;
} LVKW_TransientPool;

/**
 * @brief Initializes a transient pool to an empty state.
 */
void _lvkw_transient_pool_init(LVKW_TransientPool *pool);

/**
 * @brief Frees all resources associated with the pool, including all buckets.
 */
void _lvkw_transient_pool_destroy(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base);

/**
 * @brief Allocates transient memory. 
 * Small allocations go into buckets, large ones are allocated directly.
 * @return A pointer that is stable until the next clear/rotation.
 */
void *_lvkw_transient_pool_alloc(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, size_t size);

/**
 * @brief Interns a string into the transient pool.
 * @return A stable pointer to the copied string.
 */
const char *_lvkw_transient_pool_intern(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, const char *str);

/**
 * @brief Interns a string with a known length into the transient pool.
 * @return A stable pointer to the copied string.
 */
const char *_lvkw_transient_pool_intern_sized(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base, const char *str, size_t len);

/**
 * @brief Resets the pool for the next cycle.
 * Buckets are kept but reset to zero usage. Direct allocations are freed.
 */
void _lvkw_transient_pool_clear(LVKW_TransientPool *pool, LVKW_Context_Base *ctx_base);

#ifdef __cplusplus
}
#endif

#endif // LVKW_TRANSIENT_POOL_H_INCLUDED
