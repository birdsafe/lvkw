// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CURSOR_H_INCLUDED
#define LVKW_CURSOR_H_INCLUDED

#include "lvkw-core.h"

/**
 * @file lvkw-cursor.h
 * @brief Hardware cursor management.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Standard OS cursor shapes. */
typedef enum LVKW_CursorShape {
  LVKW_CURSOR_SHAPE_DEFAULT = 1,       ///< Standard arrow.
  LVKW_CURSOR_SHAPE_HELP = 2,          ///< Arrow with question mark.
  LVKW_CURSOR_SHAPE_POINTER = 3,       ///< Hand (typically for links).
  LVKW_CURSOR_SHAPE_WAIT = 4,          ///< Busy indicator.
  LVKW_CURSOR_SHAPE_CROSSHAIR = 5,     ///< Precision cross.
  LVKW_CURSOR_SHAPE_TEXT = 6,          ///< I-beam for text selection.
  LVKW_CURSOR_SHAPE_MOVE = 7,          ///< 4-way arrow for movement.
  LVKW_CURSOR_SHAPE_NOT_ALLOWED = 8,   ///< Slashed circle (forbidden).
  LVKW_CURSOR_SHAPE_EW_RESIZE = 9,     ///< Horizontal resize.
  LVKW_CURSOR_SHAPE_NS_RESIZE = 10,    ///< Vertical resize.
  LVKW_CURSOR_SHAPE_NESW_RESIZE = 11,  ///< Diagonal resize (/).
  LVKW_CURSOR_SHAPE_NWSE_RESIZE = 12,  ///< Diagonal resize (\).
} LVKW_CursorShape;

/** @brief Runtime status flags for a cursor. */
typedef enum LVKWCursorFlags {
  LVKW_CURSOR_FLAG_SYSTEM = 1 << 0,  ///< Cursor is a standard system shape and
                                     ///< should not be destroyed by the user.
} LVKWCursorFlags;

/** @brief Opaque handle representing a hardware-accelerated cursor. */
typedef struct LVKW_Cursor {
  uint32_t flags;
} LVKW_Cursor;

/** @brief Parameters for creating a custom cursor from pixels. */
typedef struct LVKW_CursorCreateInfo {
  LVKW_PixelVec size;      ///< Dimensions of the cursor image.
  LVKW_PixelVec hotSpot;   ///< Pixel offset from top-left (0,0) to the interaction point.
  const uint32_t *pixels;  ///< Raw RGBA8888 pixel data (row-major).
} LVKW_CursorCreateInfo;

/**
 * @brief Retrieves a handle to a standard system cursor.
 * @note **Ownership:** These handles are managed by the context and should NOT
 * be destroyed by the user.
 * @note Threading: callable from any thread. Synchronize context lifetime with
 * @ref lvkw_ctx_destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape,
                                                 LVKW_Cursor **out_cursor);

/**
 * @brief Creates a custom hardware cursor from pixel data.
 * @note **Ownership:** The returned cursor MUST be destroyed with @ref
 * lvkw_cursor_destroy.
 * @note The pixel data is copied; the caller can discard it after this call.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_ctx_createCursor(LVKW_Context *ctx,
                                            const LVKW_CursorCreateInfo *create_info,
                                            LVKW_Cursor **out_cursor);

/**
 * @brief Destroys a custom cursor and releases OS resources.
 * @note It is safe to destroy a cursor that is currently in use by one or more
 * windows. The windows will continue to use the cursor until it is changed or
 * the window is destroyed.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_cursor_destroy(LVKW_Cursor *cursor);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CURSOR_H_INCLUDED
