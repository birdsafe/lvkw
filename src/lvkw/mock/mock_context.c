#include <stdlib.h>
#include <string.h>

#include "lvkw_api_checks.h"
#include "lvkw_mock_internal.h"

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

LVKW_Status lvkw_context_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  LVKW_Allocator allocator = {.alloc = _lvkw_default_alloc, .free = _lvkw_default_free};

  if (create_info->allocator.alloc) {
    allocator = create_info->allocator;
  }

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)allocator.alloc(sizeof(LVKW_Context_Mock), create_info->user_data);
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_OUT_OF_MEMORY,
                                    "Failed to allocate storage for context");
    return LVKW_ERROR_NOOP;
  }

  memset(ctx, 0, sizeof(LVKW_Context_Mock));
  ctx->base.alloc_cb = allocator;
  ctx->base.diagnosis_cb = create_info->diagnosis_callback;
  ctx->base.diagnosis_user_data = create_info->diagnosis_user_data;
  ctx->base.user_data = create_info->user_data;

  LVKW_Result res = lvkw_event_queue_init(&ctx->base, &ctx->event_queue, 64, 256);
  if (res != LVKW_OK) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Failed to allocate event queue pool");
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR_NOOP;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  return LVKW_OK;
}

void lvkw_context_destroy_Mock(LVKW_Context *ctx_handle) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  // Destroy all windows in list

  while (ctx->window_list_head) {
    lvkw_window_destroy_Mock((LVKW_Window *)ctx->window_list_head);
  }

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);

  lvkw_context_free(&ctx->base, ctx);
}

void *lvkw_context_getUserData_Mock(const LVKW_Context *ctx_handle) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  return ctx->base.user_data;
}

void lvkw_context_getVulkanInstanceExtensions_Mock(const LVKW_Context *ctx_handle, uint32_t *count,
                                                   const char **out_extensions) {
  (void)ctx_handle;

  if (count) {
    *count = 0;
  }

  (void)out_extensions;
}

LVKW_ContextResult lvkw_context_pollEvents_Mock(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback,

                                                 void *userdata) {
  return lvkw_context_waitEvents_Mock(ctx_handle, 0, event_mask, callback, userdata);
}

LVKW_ContextResult lvkw_context_waitEvents_Mock(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                                 LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_Event evt;

  while (lvkw_event_queue_pop(&ctx->event_queue, event_mask, &evt)) {
    if (evt.type == LVKW_EVENT_TYPE_WINDOW_READY) {
      ((LVKW_Window_Base *)evt.window_ready.window)->is_ready = true;
    }

    callback(&evt, userdata);
  }

  return LVKW_OK;
}

LVKW_Status lvkw_context_setIdleTimeout_Mock(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  ctx->idle_timeout_ms = timeout_ms;

  return LVKW_OK;
}
