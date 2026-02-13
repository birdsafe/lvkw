#include "lvkw/lvkw-context.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline LVKW_Status lvkw_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {0};
  attrs.idle_timeout_ms = timeout_ms;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs);
}

static inline LVKW_Status lvkw_ctx_setIdleInhibition(LVKW_Context *ctx, bool enabled) {
  LVKW_ContextAttributes attrs = {0};
  attrs.inhibit_idle = enabled;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs);
}
static inline LVKW_Status lvkw_ctx_setDiagnosticCallback(LVKW_Context *ctx, LVKW_DiagnosticCallback callback,
                                                         void *userdata) {
  LVKW_ContextAttributes attrs = {0};
  attrs.diagnostic_cb = callback;
  attrs.diagnostic_userdata = userdata;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_DIAGNOSTICS, &attrs);
}

#ifdef __cplusplus
}
#endif