// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DATA_H_INCLUDED
#define LVKW_DATA_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lvkw/c/display.h"

/**
 * @file data.h
 * @brief Clipboard and drag-and-drop data APIs.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Supported Drag and Drop actions for OS feedback. */
typedef enum LVKW_DndAction {
  LVKW_DND_ACTION_NONE = 0,
  LVKW_DND_ACTION_COPY = 1,
  LVKW_DND_ACTION_MOVE = 2,
  LVKW_DND_ACTION_LINK = 3,
} LVKW_DndAction;

/** @brief System-managed data exchange targets. */
typedef enum LVKW_DataExchangeTarget {
  LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD = 0,
  LVKW_DATA_EXCHANGE_TARGET_PRIMARY = 1,
} LVKW_DataExchangeTarget;

/** @brief Container for arbitrary data-exchange payloads. */
typedef struct LVKW_DataBuffer {
  const char *mime_type;
  const void *data;
  size_t size;
} LVKW_DataBuffer;

/** @brief Backward-compatible alias. */
typedef LVKW_DataBuffer LVKW_ClipboardData;

LVKW_COLD LVKW_Status lvkw_data_pushText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                         const char *text);

LVKW_COLD LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                         const char **out_text);

LVKW_COLD LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                         const LVKW_DataBuffer *data, uint32_t count);

LVKW_COLD LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                         const char *mime_type, const void **out_data,
                                         size_t *out_size);

LVKW_COLD LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window,
                                                    LVKW_DataExchangeTarget target,
                                                    const char ***out_mime_types,
                                                    uint32_t *count);

LVKW_COLD LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window, const char *text);

LVKW_COLD LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text);

LVKW_COLD LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window,
                                                 const LVKW_ClipboardData *data,
                                                 uint32_t count);

LVKW_COLD LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type,
                                                 const void **out_data, size_t *out_size);

LVKW_COLD LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window,
                                                      const char ***out_mime_types,
                                                      uint32_t *count);

static inline LVKW_Status lvkw_data_setWindowAcceptDnd(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.accept_dnd = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_ACCEPT_DND, &attrs);
}

static inline LVKW_Status lvkw_data_setWindowPrimarySelection(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.primary_selection = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_PRIMARY_SELECTION, &attrs);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_DATA_H_INCLUDED
