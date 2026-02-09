#ifndef LVKW_INTERNAL_H_INCLUDED
#define LVKW_INTERNAL_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "lvkw/lvkw.h"

typedef struct LVKW_External_Lib_Base {
  void *handle;
  bool available;
} LVKW_External_Lib_Base;

// LVKW_ENABLE_DEBUG_DIAGNOSIS implies LVKW_ENABLE_DIAGNOSIS
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
#ifndef LVKW_ENABLE_DIAGNOSIS
#define LVKW_ENABLE_DIAGNOSIS
#endif
#endif

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    typeof(lvkw_context_destroy) *destroy;
    typeof(lvkw_context_getVulkanInstanceExtensions) *get_vulkan_instance_extensions;
    typeof(lvkw_context_pollEvents) *poll_events;
    typeof(lvkw_context_setIdleTimeout) *set_idle_timeout;
    typeof(lvkw_context_getUserData) *get_user_data;
  } context;

  struct {
    typeof(lvkw_window_create) *create;
    typeof(lvkw_window_destroy) *destroy;
    typeof(lvkw_window_createVkSurface) *create_vk_surface;
    typeof(lvkw_window_getFramebufferSize) *get_framebuffer_size;
    typeof(lvkw_window_getUserData) *get_user_data;
    typeof(lvkw_window_setFullscreen) *set_fullscreen;
    typeof(lvkw_window_setCursorMode) *set_cursor_mode;
    typeof(lvkw_window_setCursorShape) *set_cursor_shape;
    typeof(lvkw_window_requestFocus) *request_focus;
  } window;
} LVKW_Backend;
#endif

typedef struct LVKW_Context_Base {
#ifdef LVKW_INDIRECT_BACKEND
  const LVKW_Backend *backend;
#endif
  LVKW_Allocator alloc_cb;
  LVKW_DiagnosisCallback diagnosis_cb;
  void *diagnosis_user_data;
  void *user_data;
  bool is_lost;
} LVKW_Context_Base;

typedef struct LVKW_Window_Base {
#ifdef LVKW_INDIRECT_BACKEND
  const LVKW_Backend *backend;
#endif
  LVKW_Context_Base *ctx_base;
  void *user_data;
  bool is_lost;
  bool is_ready;
} LVKW_Window_Base;

// Diagnosis Management
#ifdef LVKW_ENABLE_DIAGNOSIS
/* Diagnosis reporting helpers */
void _lvkw_report_bootstrap_diagnosis_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnosis diagnosis,
                                               const char *message);

#define LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, diagnosis, msg) \
  _lvkw_report_bootstrap_diagnosis_internal(create_info, diagnosis, msg)

#define LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, diagnosis, msg) \
  lvkw_reportDiagnosis((const LVKW_Context *)(ctx_base), NULL, (diagnosis), (msg))

#define LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg)                                           \
  lvkw_reportDiagnosis(                                                                                   \
      (window_base) ? (const LVKW_Context *)(((const LVKW_Window_Base *)(window_base))->ctx_base) : NULL, \
      (const LVKW_Window *)(window_base), (diagnosis), (msg))
#else
#define LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, diagnosis, msg) (void)0
#define LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, diagnosis, msg) (void)0
#define LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg) (void)0
#endif

#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
#define _lvkw_debug_ctx_check(ctx_base, cond, diagnosis, msg)                         \
  if (!(cond)) {                                                                      \
    lvkw_reportDiagnosis((const LVKW_Context *)(ctx_base), NULL, (diagnosis), (msg)); \
    abort();                                                                          \
  }
#define _lvkw_debug_wind_check(window_base, cond, diagnosis, msg) \
  if (!(cond)) {                                                  \
    LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg);      \
    abort();                                                      \
  }
#else
#define _lvkw_debug_ctx_check(ctx_base, cond, diagnosis, msg) (void)0
#define _lvkw_debug_wind_check(window_base, cond, diagnosis, msg) (void)0
#endif

// Be careful with these!
// The condition will only be evaluated when debug diagnosis is enabled.
#define LVKW_CTX_ASSERT_ARG(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg)
#define LVKW_WND_ASSERT_ARG(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg)

#define LVKW_CTX_ASSERT_PRECONDITION(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg)
#define LVKW_WND_ASSERT_PRECONDITION(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg)

#define LVKW_CTX_ASSUME(ctx_base, cond, msg) _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_INTERNAL, msg)
#define LVKW_WND_ASSUME(window_base, cond, msg) _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_INTERNAL, msg)

/* Memory allocation helpers */
static inline void *lvkw_alloc(const LVKW_Allocator *alloc_cb, void *userdata, size_t size) {
  return alloc_cb->alloc(size, userdata);
}

static inline void lvkw_free(const LVKW_Allocator *alloc_cb, void *userdata, void *ptr) {
  if (ptr) alloc_cb->free(ptr, userdata);
}

static inline void *lvkw_context_alloc(LVKW_Context_Base *ctx_base, size_t size) {
  void *ptr = ctx_base->alloc_cb.alloc(size, ctx_base->user_data);
  if (!ptr) {
    LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Out of memory");
  }
  return ptr;
}

static inline void lvkw_context_free(LVKW_Context_Base *ctx_base, void *ptr) {
  lvkw_free(&ctx_base->alloc_cb, ctx_base->user_data, ptr);
}

#endif
