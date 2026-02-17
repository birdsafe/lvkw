// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_MOCK_INTERNAL_H_INCLUDED
#define LVKW_MOCK_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "event_queue.h"
#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LVKW_Window_Mock LVKW_Window_Mock;

#define LVKW_MOCK_MAX_MODES_PER_MONITOR 16

typedef struct LVKW_Monitor_Mock {
  LVKW_Monitor_Base base;
  LVKW_VideoMode modes[LVKW_MOCK_MAX_MODES_PER_MONITOR];
  uint32_t mode_count;
} LVKW_Monitor_Mock;

typedef struct LVKW_Cursor_Mock {
  LVKW_Cursor_Base base;
  LVKW_CursorShape shape;
} LVKW_Cursor_Mock;

typedef struct LVKW_Context_Mock {
  LVKW_Context_Base base;
  bool inhibit_idle;
  LVKW_Cursor_Mock standard_cursors[13];
} LVKW_Context_Mock;

typedef struct LVKW_Window_Mock {
  LVKW_Window_Base base;

  LVKW_LogicalVec size;
  LVKW_PixelVec framebuffer_size;
  bool is_fullscreen;
  bool is_maximized;
  LVKW_CursorMode cursor_mode;
  LVKW_Cursor *cursor;
  char *title;
  char *app_id;
  bool transparent;
  bool accept_dnd;
  LVKW_TextInputType text_input_type;
  LVKW_LogicalRect text_input_rect;
  LVKW_Monitor *monitor;
} LVKW_Window_Mock;

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_Mock(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx, uint32_t *count, const char *const **out_extensions);
LVKW_Status lvkw_ctx_pumpEvents_Mock(LVKW_Context *ctx, uint32_t timeout_ms);
LVKW_Status lvkw_ctx_commitEvents_Mock(LVKW_Context *ctx);
LVKW_Status lvkw_ctx_postEvent_Mock(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                    const LVKW_Event *evt);
LVKW_Status lvkw_ctx_scanEvents_Mock(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                     void *userdata);
LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_Mock(LVKW_Context *ctx, LVKW_MonitorRef **out_refs, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_Mock(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                          LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_getMetrics_Mock(LVKW_Context *ctx_handle, LVKW_MetricsCategory category, void *out_data,
                                        bool reset);

                                          #ifdef LVKW_ENABLE_CONTROLLER
typedef struct LVKW_Controller_Mock {
  LVKW_Controller_Base base;
  LVKW_Scalar haptic_levels[LVKW_CTRL_HAPTIC_STANDARD_COUNT];
} LVKW_Controller_Mock;
#endif
LVKW_Status lvkw_ctx_createWindow_Mock(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                       LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_Mock(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Mock(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_Mock(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_Mock(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Mock(LVKW_Window *window);
LVKW_Status lvkw_wnd_setClipboardText_Mock(LVKW_Window *window, const char *text);
LVKW_Status lvkw_wnd_getClipboardText_Mock(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData_Mock(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData_Mock(LVKW_Window *window, const char *mime_type, const void **out_data,
                                           size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes_Mock(LVKW_Window *window, const char ***out_mime_types, uint32_t *count);

LVKW_Status lvkw_ctx_getStandardCursor_Mock(LVKW_Context *ctx, LVKW_CursorShape shape,
                                            LVKW_Cursor **out_cursor);
LVKW_Status lvkw_ctx_createCursor_Mock(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                       LVKW_Cursor **out_cursor);
LVKW_Status lvkw_cursor_destroy_Mock(LVKW_Cursor *cursor);

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_ctrl_create_Mock(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller);
LVKW_Status lvkw_ctrl_getInfo_Mock(LVKW_Controller *controller, LVKW_CtrlInfo *out_info);
LVKW_Status lvkw_ctrl_setHapticLevels_Mock(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                           const LVKW_Scalar *intensities);
#endif

#define LVKW_BACKEND_MOCK 4

#ifdef __cplusplus
}
#endif

#endif
