// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_MEM_INTERNAL_H_INCLUDED
#define LVKW_MEM_INTERNAL_H_INCLUDED

#include <stddef.h>
#include <string.h>

#include "lvkw_diagnostic_internal.h"
#include "lvkw_types_internal.h"

/* Memory allocation helpers */
static inline void *lvkw_alloc(const LVKW_Allocator *alloc_cb, void *userdata, size_t size) {
  return alloc_cb->alloc_cb(size, userdata);
}

static inline void lvkw_free(const LVKW_Allocator *alloc_cb, void *userdata, void *ptr) {
  if (ptr) alloc_cb->free_cb(ptr, userdata);
}

static inline void *lvkw_realloc(const LVKW_Allocator *alloc_cb, void *userdata, void *ptr,
                                 size_t old_size, size_t new_size) {
  if (alloc_cb->realloc_cb) {
    return alloc_cb->realloc_cb(ptr, new_size, userdata);
  }
  else {
    if (new_size == 0) {
      lvkw_free(alloc_cb, userdata, ptr);
      return NULL;
    }

    void *new_ptr = lvkw_alloc(alloc_cb, userdata, new_size);
    if (new_ptr && ptr) {
      if (old_size > 0) {
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);
      }
      lvkw_free(alloc_cb, userdata, ptr);
    }
    return new_ptr;
  }
}

static inline void *lvkw_context_alloc(LVKW_Context_Base *ctx_base, size_t size) {
  void *ptr = ctx_base->prv.alloc_cb.alloc_cb(size, ctx_base->prv.allocator_userdata);
  if (!ptr) {
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
  }
  return ptr;
}

static inline void lvkw_context_free(LVKW_Context_Base *ctx_base, void *ptr) {
  lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->prv.allocator_userdata, ptr);
}

static inline void *lvkw_context_alloc_aligned64(LVKW_Context_Base *ctx_base, size_t size) {
  const size_t alignment = 64;
  /* Allocate extra space for alignment and to store the original pointer */
  void *ptr = lvkw_context_alloc(ctx_base, size + alignment + sizeof(void *));
  if (!ptr) return NULL;

  /* Calculate aligned address */
  size_t addr = (size_t)ptr + sizeof(void *);
  void *aligned_ptr = (void *)((addr + alignment - 1) & ~(alignment - 1));

  /* Store original pointer immediately before the aligned pointer */
  ((void **)aligned_ptr)[-1] = ptr;

  return aligned_ptr;
}

static inline void lvkw_context_free_aligned64(LVKW_Context_Base *ctx_base, void *ptr) {
  if (ptr) {
    /* Retrieve original pointer stored before the aligned address */
    void *original_ptr = ((void **)ptr)[-1];
    lvkw_context_free(ctx_base, original_ptr);
  }
}

static inline void *lvkw_context_realloc(LVKW_Context_Base *ctx_base, void *ptr, size_t old_size,
                                         size_t new_size) {
  void *new_ptr = lvkw_realloc(&ctx_base->prv.alloc_cb, ctx_base->prv.allocator_userdata, ptr,
                               old_size, new_size);
  if (!new_ptr && new_size > 0) {
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
  }

  return new_ptr;
}

#endif  // LVKW_MEM_INTERNAL_H_INCLUDED
