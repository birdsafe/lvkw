#ifndef LVKW_MOCK_INTERNAL_H_INCLUDED
#define LVKW_MOCK_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"

typedef struct LVKW_Window_Mock LVKW_Window_Mock;

#define LVKW_MOCK_MAX_MONITORS 8
#define LVKW_MOCK_MAX_MODES_PER_MONITOR 16

typedef struct LVKW_Context_Mock {
  LVKW_Context_Base base;
  LVKW_EventQueue event_queue;
  uint32_t idle_timeout_ms;
  bool inhibit_idle;

  /* Mock monitor state */
  LVKW_MonitorInfo monitors[LVKW_MOCK_MAX_MONITORS];
  uint32_t monitor_count;

  /* Mock video modes per monitor (indexed by position in monitors array) */
  LVKW_VideoMode monitor_modes[LVKW_MOCK_MAX_MONITORS][LVKW_MOCK_MAX_MODES_PER_MONITOR];
  uint32_t monitor_mode_counts[LVKW_MOCK_MAX_MONITORS];
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
} LVKW_Window_Mock;

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_Mock(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx, uint32_t *count, const char *const **out_extensions);
LVKW_Status lvkw_ctx_pollEvents_Mock(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                     void *userdata);
LVKW_Status lvkw_ctx_waitEvents_Mock(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                     LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_Mock(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_Mock(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                          LVKW_VideoMode *out_modes, uint32_t *count);

LVKW_Status lvkw_ctx_createWindow_Mock(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                       LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_Mock(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Mock(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_Mock(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_Mock(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Mock(LVKW_Window *window);

LVKW_Cursor *lvkw_ctx_getStandardCursor_Mock(LVKW_Context *ctx, LVKW_CursorShape shape);
LVKW_Status lvkw_ctx_createCursor_Mock(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                       LVKW_Cursor **out_cursor);
LVKW_Status lvkw_cursor_destroy_Mock(LVKW_Cursor *cursor);

#define LVKW_BACKEND_MOCK 4

#endif
