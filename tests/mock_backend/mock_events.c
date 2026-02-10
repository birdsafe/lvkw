#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"

void lvkw_mock_pushEvent(LVKW_Context *handle, const LVKW_Event *evt) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;

  LVKW_CTX_ASSERT_ARG(handle, ctx != NULL, "Context handle must not be NULL");
  LVKW_CTX_ASSERT_ARG(handle, evt != NULL, "Event pointer must not be NULL");

  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, evt);
}