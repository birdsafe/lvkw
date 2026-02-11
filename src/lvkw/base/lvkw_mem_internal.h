#ifndef LVKW_MEM_INTERNAL_H_INCLUDED
#define LVKW_MEM_INTERNAL_H_INCLUDED

#include <stddef.h>
#include <string.h>
#include "lvkw_types_internal.h"
#include "lvkw_diag_internal.h"

/* Memory allocation helpers */
static inline void *lvkw_alloc(const LVKW_Allocator *alloc_cb, void *userdata, size_t size) {
  return alloc_cb->alloc_cb(size, userdata);
}

static inline void lvkw_free(const LVKW_Allocator *alloc_cb, void *userdata, void *ptr) {
  if (ptr) alloc_cb->free_cb(ptr, userdata);
}

static inline void *lvkw_context_alloc(LVKW_Context_Base *ctx_base, size_t size) {
  void *ptr = ctx_base->prv.alloc_cb.alloc_cb(size, ctx_base->prv.allocator_userdata);
  if (!ptr) {
    LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Out of memory");
  }
  return ptr;
}

static inline void lvkw_context_free(LVKW_Context_Base *ctx_base, void *ptr) {
  lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->prv.allocator_userdata, ptr);
}

static inline void *lvkw_context_realloc(LVKW_Context_Base *ctx_base, void *ptr, size_t old_size, size_t new_size) {
  if (new_size == 0) {
    lvkw_context_free(ctx_base, ptr);
    return NULL;
  }

  void *new_ptr = lvkw_context_alloc(ctx_base, new_size);
  if (!new_ptr) {
    return NULL;
  }

  if (ptr && old_size > 0) {
    size_t copy_size = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, ptr, copy_size);
    lvkw_context_free(ctx_base, ptr);
  }

  return new_ptr;
}

#endif // LVKW_MEM_INTERNAL_H_INCLUDED
