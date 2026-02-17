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

/** @brief Container for arbitrary clipboard data. */
typedef struct LVKW_ClipboardData {
  const char *mime_type;
  const void *data;
  size_t size;
} LVKW_ClipboardData;

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

LVKW_COLD LVKW_Status lvkw_data_setWindowAcceptDnd(LVKW_Window *window, bool enabled);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_DATA_H_INCLUDED
