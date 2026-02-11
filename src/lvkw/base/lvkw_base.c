#include <stddef.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw/details/lvkw_version.h"

LVKW_Version lvkw_getVersion(void) {
  return (LVKW_Version){
      .major = LVKW_VERSION_MAJOR,
      .minor = LVKW_VERSION_MINOR,
      .patch = LVKW_VERSION_PATCH,
  };
}

void _lvkw_context_init_base(LVKW_Context_Base *ctx_base, const LVKW_ContextCreateInfo *create_info) {
  memset(ctx_base, 0, sizeof(*ctx_base));
  ctx_base->pub.userdata = create_info->userdata;
  ctx_base->prv.diagnosis_cb = create_info->attributes.diagnosis_cb;
  ctx_base->prv.diagnosis_userdata = create_info->attributes.diagnosis_userdata;
  ctx_base->prv.allocator_userdata = create_info->userdata;
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
  ctx_base->prv.creator_thread = thrd_current();
#endif
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

#ifdef LVKW_ENABLE_DIAGNOSIS
void _lvkw_report_bootstrap_diagnosis_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnosis diagnosis,
                                               const char *message) {
  if (create_info && create_info->attributes.diagnosis_cb) {
    LVKW_DiagnosisInfo info = {
        .diagnosis = diagnosis,
        .message = message,
        .context = NULL,
        .window = NULL,
    };
    create_info->attributes.diagnosis_cb(&info, create_info->attributes.diagnosis_userdata);
  }
}

void _lvkw_reportDiagnosis(LVKW_Context *ctx_handle, LVKW_Window *window_handle, LVKW_Diagnosis diagnosis,
                           const char *message) {
  if (!ctx_handle) return;
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  if (ctx_base->prv.diagnosis_cb) {
    LVKW_DiagnosisInfo info = {
        .diagnosis = diagnosis,
        .message = message,
        .context = ctx_handle,
        .window = window_handle,
    };
    ctx_base->prv.diagnosis_cb(&info, ctx_base->prv.diagnosis_userdata);
  }
}
#else
void _lvkw_reportDiagnosis(LVKW_Context *ctx_handle, LVKW_Window *window_handle, LVKW_Diagnosis diagnosis,
                           const char *message) {
  (void)ctx_handle;
  (void)window_handle;
  (void)diagnosis;
  (void)message;
}
#endif

LVKW_Context *lvkw_wnd_getContext(LVKW_Window *window_handle) {
  if (!window_handle) return NULL;
  return (LVKW_Context *)((const LVKW_Window_Base *)window_handle)->prv.ctx_base;
}
