#ifndef LVKW_TYPES_INTERNAL_H_INCLUDED
#define LVKW_TYPES_INTERNAL_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "lvkw/lvkw.h"
#include "lvkw_string_cache.h"
#include "lvkw_thread_internal.h"

// Forward declaration of LVKW_Backend to allow use in Context/Window
struct LVKW_Backend;

typedef struct LVKW_External_Lib_Base {
  void *handle;
  bool available;
} LVKW_External_Lib_Base;

typedef struct LVKW_Context_Base {
  LVKW_Context pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Allocator alloc_cb;
    void *allocator_userdata;
    LVKW_DiagnosisCallback diagnosis_cb;
    void *diagnosis_userdata;
    struct LVKW_Window_Base *window_list;
    LVKW_StringCache string_cache;
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
    LVKW_ThreadId creator_thread;
#endif
  } prv;
} LVKW_Context_Base;

typedef struct LVKW_Window_Base {
  LVKW_Window pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
    struct LVKW_Window_Base *next;
  } prv;
} LVKW_Window_Base;

/* Shared internal helpers */
void _lvkw_context_init_base(LVKW_Context_Base *ctx_base, const LVKW_ContextCreateInfo *create_info);
void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base);

typedef struct _LVKW_EventTuning {
  uint32_t initial_capacity;
  uint32_t max_capacity;
  double growth_factor;
} _LVKW_EventTuning;

_LVKW_EventTuning _lvkw_get_event_tuning(const LVKW_ContextCreateInfo *create_info);
void _lvkw_context_mark_lost(LVKW_Context_Base *ctx_base);
void _lvkw_window_list_add(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);
void _lvkw_window_list_remove(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);

#endif  // LVKW_TYPES_INTERNAL_H_INCLUDED
