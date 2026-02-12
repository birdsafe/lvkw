#include <stddef.h>
#include <string.h>

#include "lvkw/details/lvkw_version.h"
#include "lvkw/lvkw-tuning.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_internal.h"

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  LVKW_API_VALIDATE(createContext, create_info, out_context);

  LVKW_ContextCreateInfo create_info_copy;
  const LVKW_ContextTuning tuning_defaults = LVKW_CONTEXT_TUNING_DEFAULT;

  if (!create_info->tuning) {
    create_info_copy = *create_info;
    create_info_copy.tuning = &tuning_defaults;
    create_info = &create_info_copy;
  }

  return _lvkw_createContext_impl(create_info, out_context);
}

LVKW_Version lvkw_getVersion(void) {
  return (LVKW_Version){
      .major = LVKW_VERSION_MAJOR,
      .minor = LVKW_VERSION_MINOR,
      .patch = LVKW_VERSION_PATCH,
  };
}

LVKW_Status lvkw_wnd_getContext(LVKW_Window *window_handle, LVKW_Context **out_context) {
  LVKW_API_VALIDATE(wnd_getContext, window_handle, out_context);

  *out_context = (LVKW_Context *)((const LVKW_Window_Base *)window_handle)->prv.ctx_base;
  return LVKW_SUCCESS;
}

void _lvkw_context_init_base(LVKW_Context_Base *ctx_base, const LVKW_ContextCreateInfo *create_info) {
  memset(ctx_base, 0, sizeof(*ctx_base));
  ctx_base->pub.userdata = create_info->userdata;
  ctx_base->prv.diagnostic_cb = create_info->attributes.diagnostic_cb;
  ctx_base->prv.diagnostic_userdata = create_info->attributes.diagnostic_userdata;
  ctx_base->prv.allocator_userdata = create_info->userdata;
  _lvkw_string_cache_init(&ctx_base->prv.string_cache);
#if LVKW_API_VALIDATION > 0
  ctx_base->prv.creator_thread = _lvkw_get_current_thread_id();
#endif
}

void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base) {
  _lvkw_string_cache_destroy(&ctx_base->prv.string_cache, ctx_base);
}

void _lvkw_context_mark_lost(LVKW_Context_Base *ctx_base) {
  if (!ctx_base || (ctx_base->pub.flags & LVKW_CTX_STATE_LOST)) return;
  ctx_base->pub.flags |= LVKW_CTX_STATE_LOST;

  LVKW_Window_Base *curr = ctx_base->prv.window_list;
  while (curr) {
    curr->pub.flags |= LVKW_WND_STATE_LOST;
    curr = curr->prv.next;
  }
}

void _lvkw_window_list_add(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base) {
  window_base->prv.next = ctx_base->prv.window_list;
  ctx_base->prv.window_list = window_base;
}

void _lvkw_window_list_remove(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base) {
  LVKW_Window_Base **curr = &ctx_base->prv.window_list;
  while (*curr) {
    if (*curr == window_base) {
      *curr = window_base->prv.next;
      break;
    }
    curr = &((*curr)->prv.next);
  }
}

#ifdef LVKW_ENABLE_DIAGNOSTICS
void _lvkw_report_bootstrap_diagnostic_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnostic diagnostic,
                                                const char *message) {
  if (create_info && create_info->attributes.diagnostic_cb) {
    LVKW_DiagnosticInfo info = {
        .diagnostic = diagnostic,
        .message = message,
        .context = NULL,
        .window = NULL,
    };
    create_info->attributes.diagnostic_cb(&info, create_info->attributes.diagnostic_userdata);
  }
}

void _lvkw_reportDiagnostic(LVKW_Context *ctx_handle, LVKW_Window *window_handle, LVKW_Diagnostic diagnostic,
                            const char *message) {
  if (!ctx_handle) return;
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  if (ctx_base->prv.diagnostic_cb) {
    LVKW_DiagnosticInfo info = {
        .diagnostic = diagnostic,
        .message = message,
        .context = ctx_handle,
        .window = window_handle,
    };
    ctx_base->prv.diagnostic_cb(&info, ctx_base->prv.diagnostic_userdata);
  }
}
#else
void _lvkw_reportDiagnostic(LVKW_Context *ctx_handle, LVKW_Window *window_handle, LVKW_Diagnostic diagnostic,
                            const char *message) {
  (void)ctx_handle;
  (void)window_handle;
  (void)diagnostic;
  (void)message;
}
#endif
