#include <stdlib.h>
#include <string.h>

#include "lvkw_api_checks.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                 const LVKW_ContextAttributes *attributes);

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  LVKW_Allocator allocator = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};

  if (create_info->allocator.alloc_cb) {
    allocator = create_info->allocator;
  }

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)lvkw_alloc(&allocator, create_info->userdata, sizeof(LVKW_Context_Mock));
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_OUT_OF_MEMORY,
                                    "Failed to allocate storage for context");
    return LVKW_ERROR;
  }

  _lvkw_context_init_base(&ctx->base, create_info);
  ctx->base.prv.alloc_cb = allocator;

  _LVKW_EventTuning tuning = _lvkw_get_event_tuning(create_info);
  LVKW_Status res =
      lvkw_event_queue_init(&ctx->base, &ctx->event_queue, tuning.initial_capacity, tuning.max_capacity, tuning.growth_factor);
  if (res != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Failed to allocate event queue pool");
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  // Apply initial attributes
  lvkw_ctx_update_Mock((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;
}

void lvkw_ctx_destroy_Mock(LVKW_Context *ctx_handle) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  // Destroy all windows in list
  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_Mock((LVKW_Window *)ctx->base.prv.window_list);
  }

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);

  lvkw_context_free(&ctx->base, ctx);
}

const char *const *lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx_handle, uint32_t *count) {
  (void)ctx_handle;
  static const char *extensions[] = {NULL};

  if (count) {
    *count = 0;
  }

  return extensions;
}

LVKW_Status lvkw_ctx_pollEvents_Mock(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback,

                                                 void *userdata) {
  return lvkw_ctx_waitEvents_Mock(ctx_handle, 0, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents_Mock(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                                 LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_Event evt;

  while (lvkw_event_queue_pop(&ctx->event_queue, event_mask, &evt)) {
    if (evt.type == LVKW_EVENT_TYPE_WINDOW_READY) {
      ((LVKW_Window_Base *)evt.window)->pub.flags |= LVKW_WND_STATE_READY;
    }

    callback(&evt, userdata);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                               const LVKW_ContextAttributes *attributes) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    ctx->inhibit_idle = attributes->inhibit_idle;
  }

  if (field_mask & LVKW_CTX_ATTR_DIAGNOSIS) {
    ctx->base.prv.diagnosis_cb = attributes->diagnosis_cb;
    ctx->base.prv.diagnosis_userdata = attributes->diagnosis_userdata;
  }

  return LVKW_SUCCESS;
}
