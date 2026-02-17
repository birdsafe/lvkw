// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DISPLAY_H_INCLUDED
#define LVKW_DISPLAY_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lvkw/c/core.h"

/**
 * @file display.h
 * @brief Display, monitor, window, and cursor APIs.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Standard OS cursor shapes. */
typedef enum LVKW_CursorShape {
  LVKW_CURSOR_SHAPE_DEFAULT = 1,
  LVKW_CURSOR_SHAPE_HELP = 2,
  LVKW_CURSOR_SHAPE_HAND = 3,
  LVKW_CURSOR_SHAPE_WAIT = 4,
  LVKW_CURSOR_SHAPE_CROSSHAIR = 5,
  LVKW_CURSOR_SHAPE_TEXT = 6,
  LVKW_CURSOR_SHAPE_MOVE = 7,
  LVKW_CURSOR_SHAPE_NOT_ALLOWED = 8,
  LVKW_CURSOR_SHAPE_EW_RESIZE = 9,
  LVKW_CURSOR_SHAPE_NS_RESIZE = 10,
  LVKW_CURSOR_SHAPE_NESW_RESIZE = 11,
  LVKW_CURSOR_SHAPE_NWSE_RESIZE = 12,
} LVKW_CursorShape;

/** @brief Runtime status flags for a cursor. */
typedef enum LVKW_CursorFlags {
  LVKW_CURSOR_FLAG_SYSTEM = 1 << 0,
} LVKW_CursorFlags;

/** @brief Opaque handle representing a hardware-accelerated cursor. */
typedef struct LVKW_Cursor {
  uint32_t flags;
} LVKW_Cursor;

/** @brief Parameters for creating a custom cursor from pixels. */
typedef struct LVKW_CursorCreateInfo {
  LVKW_PixelVec size;
  LVKW_PixelVec hot_spot;
  const uint32_t *pixels;
} LVKW_CursorCreateInfo;

/** @brief Runtime status flags for a monitor. */
typedef enum LVKW_MonitorFlags {
  LVKW_MONITOR_STATE_LOST = 1 << 0,
} LVKW_MonitorFlags;

typedef struct LVKW_MonitorRef LVKW_MonitorRef;

/** @brief A video mode supported by a monitor. */
typedef struct LVKW_VideoMode {
  LVKW_PixelVec size;
  uint32_t refresh_rate_mhz;
} LVKW_VideoMode;

/** @brief Persistent handle representing a physical monitor. */
typedef struct LVKW_Monitor {
  LVKW_Context *context;
  void *userdata;
  uint32_t flags;

  const char *name;
  LVKW_LogicalVec physical_size;
  LVKW_VideoMode current_mode;
  LVKW_LogicalVec logical_position;
  LVKW_LogicalVec logical_size;
  bool is_primary;
  LVKW_Scalar scale;
} LVKW_Monitor;

/** @brief Snapshot of window dimensions and position. */
typedef struct LVKW_WindowGeometry {
  LVKW_LogicalVec origin;
  LVKW_LogicalVec logical_size;
  LVKW_PixelVec pixel_size;
} LVKW_WindowGeometry;

/** @brief Runtime status flags for a window. */
typedef enum LVKW_WindowFlags {
  LVKW_WINDOW_STATE_LOST = 1 << 0,
  LVKW_WINDOW_STATE_READY = 1 << 1,
  LVKW_WINDOW_STATE_FOCUSED = 1 << 2,
  LVKW_WINDOW_STATE_MAXIMIZED = 1 << 3,
  LVKW_WINDOW_STATE_FULLSCREEN = 1 << 4,
} LVKW_WindowFlags;

/** @brief Opaque handle representing an OS-level window. */
struct LVKW_Window {
  LVKW_READONLY LVKW_Context *context;  ///< Borrowed parent pointer.
  void *userdata;         ///< User-defined. Read-Write.
  LVKW_READONLY uint32_t flags;         ///< Current state flag.
};

/** @brief Cursor visibility and constraint modes. */
typedef enum LVKW_CursorMode {
  LVKW_CURSOR_NORMAL = 0,
  LVKW_CURSOR_HIDDEN = 1,
  LVKW_CURSOR_LOCKED = 2,
} LVKW_CursorMode;

/** @brief Semantic hints for compositor optimization. */
typedef enum LVKW_ContentType {
  LVKW_CONTENT_TYPE_NONE = 0,
  LVKW_CONTENT_TYPE_PHOTO = 1,
  LVKW_CONTENT_TYPE_VIDEO = 2,
  LVKW_CONTENT_TYPE_GAME = 3,
} LVKW_ContentType;

/** @brief Semantic hints for the Input Method Editor. */
typedef enum LVKW_TextInputType {
  LVKW_TEXT_INPUT_TYPE_NONE = 0,
  LVKW_TEXT_INPUT_TYPE_TEXT,
  LVKW_TEXT_INPUT_TYPE_PASSWORD,
  LVKW_TEXT_INPUT_TYPE_EMAIL,
  LVKW_TEXT_INPUT_TYPE_NUMERIC,
} LVKW_TextInputType;

/** @brief Bitmask for selecting which attributes to update in
 * lvkw_display_updateWindow(). */
typedef enum LVKW_WindowAttributesField {
  LVKW_WINDOW_ATTR_TITLE = 1 << 0,
  LVKW_WINDOW_ATTR_LOGICAL_SIZE = 1 << 1,
  LVKW_WINDOW_ATTR_FULLSCREEN = 1 << 2,
  LVKW_WINDOW_ATTR_CURSOR_MODE = 1 << 3,
  LVKW_WINDOW_ATTR_CURSOR = 1 << 4,
  LVKW_WINDOW_ATTR_MONITOR = 1 << 5,
  LVKW_WINDOW_ATTR_MAXIMIZED = 1 << 6,
  LVKW_WINDOW_ATTR_MIN_SIZE = 1 << 7,
  LVKW_WINDOW_ATTR_MAX_SIZE = 1 << 8,
  LVKW_WINDOW_ATTR_ASPECT_RATIO = 1 << 9,
  LVKW_WINDOW_ATTR_RESIZABLE = 1 << 10,
  LVKW_WINDOW_ATTR_DECORATED = 1 << 11,
  LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH = 1 << 12,
  LVKW_WINDOW_ATTR_ACCEPT_DND = 1 << 13,
  LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE = 1 << 14,
  LVKW_WINDOW_ATTR_TEXT_INPUT_RECT = 1 << 15,
  LVKW_WINDOW_ATTR_PRIMARY_SELECTION = 1 << 16,
} LVKW_WindowAttributesField;

/** @brief Live-updatable window properties. */
typedef struct LVKW_WindowAttributes {
  const char *title;
  LVKW_LogicalVec logical_size;
  bool fullscreen;
  bool maximized;
  LVKW_CursorMode cursor_mode;
  LVKW_Cursor *cursor;
  LVKW_Monitor *monitor;
  LVKW_LogicalVec min_size;
  LVKW_LogicalVec max_size;
  LVKW_Fraction aspect_ratio;
  bool resizable;
  bool decorated;
  bool mouse_passthrough;
  bool accept_dnd;
  bool primary_selection;
  LVKW_TextInputType text_input_type;
  LVKW_LogicalRect text_input_rect;
} LVKW_WindowAttributes;

/** @brief Parameters for lvkw_display_createWindow(). */
typedef struct LVKW_WindowCreateInfo {
  LVKW_WindowAttributes attributes;
  const char *app_id;
  LVKW_ContentType content_type;
  bool transparent;
  void *userdata;
} LVKW_WindowCreateInfo;

/** @brief Default initialization macro for LVKW_WindowCreateInfo. */
#define LVKW_WINDOW_CREATE_INFO_DEFAULT                         \
  {.attributes = {.title = "LVKW Window",                       \
                  .logical_size = {800, 600},                    \
                  .fullscreen = false,                          \
                  .maximized = false,                           \
                  .cursor_mode = LVKW_CURSOR_NORMAL,            \
                  .cursor = NULL,                               \
                  .monitor = NULL,                              \
                  .min_size = {0, 0},                            \
                  .max_size = {0, 0},                            \
                  .aspect_ratio = {0, 0},                       \
                  .resizable = true,                            \
                  .decorated = true,                            \
                  .mouse_passthrough = false,                   \
                  .accept_dnd = false,                          \
                  .primary_selection = true,                    \
                  .text_input_type = LVKW_TEXT_INPUT_TYPE_NONE, \
                  .text_input_rect = {{0, 0}, {0, 0}}},         \
   .app_id = "lvkw.app",                                        \
   .content_type = LVKW_CONTENT_TYPE_NONE,                      \
   .transparent = false}

LVKW_COLD LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *context, uint32_t *out_count,
                                                    const char *const **out_extensions);

LVKW_COLD LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *context, LVKW_CursorShape shape,
                                                     LVKW_Cursor **out_cursor);

LVKW_COLD LVKW_Status lvkw_display_createCursor(LVKW_Context *context,
                                                const LVKW_CursorCreateInfo *create_info,
                                                LVKW_Cursor **out_cursor);

LVKW_COLD LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor);

LVKW_COLD LVKW_Status lvkw_display_listMonitors(LVKW_Context *context, LVKW_MonitorRef **out_refs,
                                                uint32_t *count);
LVKW_COLD LVKW_Status lvkw_display_createMonitor(LVKW_MonitorRef *monitor_ref,
                                                 LVKW_Monitor **out_monitor);
LVKW_COLD LVKW_Status lvkw_display_destroyMonitor(LVKW_Monitor *monitor);

LVKW_COLD LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *context,
                                                    const LVKW_Monitor *monitor,
                                                    LVKW_VideoMode *out_modes, uint32_t *count);

LVKW_COLD LVKW_Status lvkw_display_createWindow(LVKW_Context *context,
                                                const LVKW_WindowCreateInfo *create_info,
                                                LVKW_Window **out_window);

LVKW_COLD LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window);

LVKW_COLD LVKW_Status lvkw_display_updateWindow(LVKW_Window *window, uint32_t field_mask,
                                                const LVKW_WindowAttributes *attributes);

LVKW_COLD LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                   VkSurfaceKHR *out_surface);

LVKW_COLD LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window,
                                                     LVKW_WindowGeometry *out_geometry);

LVKW_COLD LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_DISPLAY_H_INCLUDED
