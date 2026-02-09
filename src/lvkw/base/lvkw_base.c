#include <stddef.h>

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

#ifdef LVKW_ENABLE_DIAGNOSIS
void _lvkw_report_bootstrap_diagnosis_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnosis diagnosis,
                                               const char *message) {
  if (create_info && create_info->diagnosis_callback) {
    LVKW_DiagnosisInfo info = {
        .diagnosis = diagnosis,
        .message = message,
        .context = NULL,
        .window = NULL,
    };
    create_info->diagnosis_callback(&info, create_info->diagnosis_user_data);
  }
}

void lvkw_reportDiagnosis(const LVKW_Context *ctx_handle, const LVKW_Window *window_handle, LVKW_Diagnosis diagnosis,
                          const char *message) {
  if (!ctx_handle) return;
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  if (ctx_base->diagnosis_cb) {
    LVKW_DiagnosisInfo info = {
        .diagnosis = diagnosis,
        .message = message,
        .context = ctx_handle,
        .window = window_handle,
    };
    ctx_base->diagnosis_cb(&info, ctx_base->diagnosis_user_data);
  }
}
#else
void lvkw_reportDiagnosis(const LVKW_Context *ctx_handle, const LVKW_Window *window_handle, LVKW_Diagnosis diagnosis,
                          const char *message) {
  (void)ctx_handle;
  (void)window_handle;
  (void)diagnosis;
  (void)message;
}
#endif

bool lvkw_context_isLost(const LVKW_Context *ctx_handle) {
  if (!ctx_handle) return true;
  return ((const LVKW_Context_Base *)ctx_handle)->is_lost;
}

bool lvkw_window_isLost(const LVKW_Window *window_handle) {
  if (!window_handle) return true;
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->is_lost || (window_base->ctx_base && window_base->ctx_base->is_lost);
}

bool lvkw_window_isReady(const LVKW_Window *window_handle) {
  if (!window_handle) return false;
  return ((const LVKW_Window_Base *)window_handle)->is_ready;
}

LVKW_Context *lvkw_window_getContext(const LVKW_Window *window_handle) {
  if (!window_handle) return NULL;
  return (LVKW_Context *)((const LVKW_Window_Base *)window_handle)->ctx_base;
}
