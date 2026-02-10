#include <dwmapi.h>
#include <stdlib.h>
#include <string.h>

#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

LVKW_Status lvkw_ctx_createWindow_Win32(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window_handle) {
  *out_window_handle = NULL;

  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;

  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Window_Win32));
  if (!window) {
    return LVKW_ERROR;
  }
  memset(window, 0, sizeof(LVKW_Window_Win32));

  window->base.prv.ctx_base = &ctx->base;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.size;
  window->cursor_mode = LVKW_CURSOR_NORMAL;
  window->cursor_shape = LVKW_CURSOR_SHAPE_DEFAULT;
  window->current_hcursor = LoadCursorW(NULL, IDC_ARROW);
  window->transparent = (create_info->flags & LVKW_WINDOW_TRANSPARENT) != 0;

  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD ex_style = WS_EX_APPWINDOW;

  RECT rect = {0, 0, (LONG)create_info->attributes.size.width, (LONG)create_info->attributes.size.height};
  AdjustWindowRectEx(&rect, style, FALSE, ex_style);

  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  int x = CW_USEDEFAULT;
  int y = CW_USEDEFAULT;

  // Convert title to WideChar
  int title_len = MultiByteToWideChar(CP_UTF8, 0, create_info->attributes.title, -1, NULL, 0);
  LPWSTR title_w = (LPWSTR)malloc(title_len * sizeof(WCHAR));
  MultiByteToWideChar(CP_UTF8, 0, create_info->attributes.title, -1, title_w, title_len);

  window->hwnd = CreateWindowExW(ex_style, (LPCWSTR)(uintptr_t)ctx->window_class_atom, title_w, style, x, y, width,
                                 height, NULL, NULL, ctx->hInstance, NULL);

  free(title_w);

  if (!window->hwnd) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "Failed to create window");
    lvkw_context_free(&ctx->base, window);
    return LVKW_ERROR;
  }

  if (create_info->flags & LVKW_WINDOW_TRANSPARENT) {
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(window->hwnd, &margins);
  }

  SetWindowLongPtrW(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

  // Add to list
  _lvkw_window_list_add(&ctx->base, &window->base);

  ShowWindow(window->hwnd, SW_SHOW);
  UpdateWindow(window->hwnd);

  window->base.pub.is_ready = true;

  // Emit Ready event if polling
  if (ctx->current_event_callback) {
    LVKW_Event ev = {0};
    ev.type = LVKW_EVENT_TYPE_WINDOW_READY;
    ev.window = (LVKW_Window *)window;
    ctx->current_event_callback(&ev, ctx->current_event_userdata);
  }

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

void lvkw_wnd_destroy_Win32(LVKW_Window *window_handle) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)window->base.prv.ctx_base;

  // Remove from list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  if (window->hwnd) {
    DestroyWindow(window->hwnd);
  }

  lvkw_context_free(&ctx->base, window);
}

LVKW_Status lvkw_wnd_createVkSurface_Win32(LVKW_Window *window_handle, VkInstance instance,
                                                    VkSurfaceKHR *out_surface) {
  *out_surface = VK_NULL_HANDLE;

  const LVKW_Window_Win32 *window = (const LVKW_Window_Win32 *)window_handle;
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)window->base.prv.ctx_base;

  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
      (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
  if (!vkCreateWin32SurfaceKHR) {
    LVKW_REPORT_WIND_DIAGNOSIS(&window->base, LVKW_DIAGNOSIS_VULKAN_FAILURE,
                               "vkCreateWin32SurfaceKHR function not found");
    return LVKW_ERROR;
  }

  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = ctx->hInstance;
  create_info.hwnd = window->hwnd;

  if (vkCreateWin32SurfaceKHR(instance, &create_info, NULL, out_surface) != VK_SUCCESS) {
    LVKW_REPORT_WIND_DIAGNOSIS(&window->base, LVKW_DIAGNOSIS_VULKAN_FAILURE, "vkCreateWin32SurfaceKHR failed");
    return LVKW_ERROR;
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getFramebufferSize_Win32(LVKW_Window *window_handle, LVKW_Size *out_size) {
  const LVKW_Window_Win32 *window = (const LVKW_Window_Win32 *)window_handle;

  RECT rect;
  GetClientRect(window->hwnd, &rect);
  *out_size = (LVKW_Size){(uint32_t)(rect.right - rect.left), (uint32_t)(rect.bottom - rect.top)};

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_updateAttributes_Win32(LVKW_Window *window_handle, uint32_t field_mask,
                                                 const LVKW_WindowAttributes *attributes) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  if (field_mask & LVKW_WND_ATTR_TITLE) {
    int title_len = MultiByteToWideChar(CP_UTF8, 0, attributes->title, -1, NULL, 0);
    LPWSTR title_w = (LPWSTR)malloc(title_len * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, attributes->title, -1, title_w, title_len);
    SetWindowTextW(window->hwnd, title_w);
    free(title_w);
  }

  if (field_mask & LVKW_WND_ATTR_SIZE) {
    window->size = attributes->size;

    DWORD style = GetWindowLongW(window->hwnd, GWL_STYLE);
    DWORD ex_style = GetWindowLongW(window->hwnd, GWL_EXSTYLE);

    RECT rect = {0, 0, (LONG)attributes->size.width, (LONG)attributes->size.height};
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    SetWindowPos(window->hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setFullscreen_Win32(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  if (window->is_fullscreen == enabled) return LVKW_SUCCESS;

  if (enabled) {
    window->stored_style = GetWindowLongW(window->hwnd, GWL_STYLE);
    window->stored_ex_style = GetWindowLongW(window->hwnd, GWL_EXSTYLE);
    GetWindowRect(window->hwnd, &window->stored_rect);

    HMONITOR hMonitor = MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(hMonitor, &mi);

    SetWindowLongW(window->hwnd, GWL_STYLE, window->stored_style & ~(WS_CAPTION | WS_THICKFRAME));
    SetWindowLongW(
        window->hwnd, GWL_EXSTYLE,
        window->stored_ex_style & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

    SetWindowPos(window->hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                 mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
  else {
    SetWindowLongW(window->hwnd, GWL_STYLE, window->stored_style);
    SetWindowLongW(window->hwnd, GWL_EXSTYLE, window->stored_ex_style);

    SetWindowPos(window->hwnd, NULL, window->stored_rect.left, window->stored_rect.top,
                 window->stored_rect.right - window->stored_rect.left,
                 window->stored_rect.bottom - window->stored_rect.top,
                 SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }

  window->is_fullscreen = enabled;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setCursorMode_Win32(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  if (window->cursor_mode == mode) return LVKW_SUCCESS;

  window->cursor_mode = mode;
  _lvkw_win32_update_cursor_clip(window);

  if (mode == LVKW_CURSOR_LOCKED) {
    ShowCursor(FALSE);

    // Warp to center immediately to prepare for locked movement
    if (GetFocus() == window->hwnd) {
      RECT rect;
      GetClientRect(window->hwnd, &rect);
      int centerX = (rect.right - rect.left) / 2;
      int centerY = (rect.bottom - rect.top) / 2;
      POINT pt = {centerX, centerY};
      ClientToScreen(window->hwnd, &pt);
      SetCursorPos(pt.x, pt.y);

      window->last_x = (double)centerX;
      window->last_y = (double)centerY;
      window->has_last_pos = true;
    }
  }
  else {
    ShowCursor(TRUE);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setCursorShape_Win32(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  if (window->cursor_shape == shape) return LVKW_SUCCESS;

  LPCWSTR idc_name = IDC_ARROW;
  switch (shape) {
      // ... (rest of switch) ...
    case LVKW_CURSOR_SHAPE_DEFAULT:
      idc_name = IDC_ARROW;
      break;
    case LVKW_CURSOR_SHAPE_CONTEXT_MENU:
      idc_name = IDC_ARROW;
      break;  // No direct equivalent
    case LVKW_CURSOR_SHAPE_HELP:
      idc_name = IDC_HELP;
      break;
    case LVKW_CURSOR_SHAPE_POINTER:
      idc_name = IDC_HAND;
      break;
    case LVKW_CURSOR_SHAPE_PROGRESS:
      idc_name = IDC_APPSTARTING;
      break;
    case LVKW_CURSOR_SHAPE_WAIT:
      idc_name = IDC_WAIT;
      break;
    case LVKW_CURSOR_SHAPE_CROSSHAIR:
      idc_name = IDC_CROSS;
      break;
    case LVKW_CURSOR_SHAPE_TEXT:
      idc_name = IDC_IBEAM;
      break;
    case LVKW_CURSOR_SHAPE_MOVE:
      idc_name = IDC_SIZEALL;
      break;
    case LVKW_CURSOR_SHAPE_NO_DROP:
      idc_name = IDC_NO;
      break;
    case LVKW_CURSOR_SHAPE_NOT_ALLOWED:
      idc_name = IDC_NO;
      break;
    case LVKW_CURSOR_SHAPE_E_RESIZE:
      idc_name = IDC_SIZEWE;
      break;
    case LVKW_CURSOR_SHAPE_N_RESIZE:
      idc_name = IDC_SIZENS;
      break;
    case LVKW_CURSOR_SHAPE_NE_RESIZE:
      idc_name = IDC_SIZENESW;
      break;
    case LVKW_CURSOR_SHAPE_NW_RESIZE:
      idc_name = IDC_SIZENWSE;
      break;
    case LVKW_CURSOR_SHAPE_S_RESIZE:
      idc_name = IDC_SIZENS;
      break;
    case LVKW_CURSOR_SHAPE_SE_RESIZE:
      idc_name = IDC_SIZENWSE;
      break;
    case LVKW_CURSOR_SHAPE_SW_RESIZE:
      idc_name = IDC_SIZENESW;
      break;
    case LVKW_CURSOR_SHAPE_W_RESIZE:
      idc_name = IDC_SIZEWE;
      break;
    case LVKW_CURSOR_SHAPE_EW_RESIZE:
      idc_name = IDC_SIZEWE;
      break;
    case LVKW_CURSOR_SHAPE_NS_RESIZE:
      idc_name = IDC_SIZENS;
      break;
    case LVKW_CURSOR_SHAPE_NESW_RESIZE:
      idc_name = IDC_SIZENESW;
      break;
    case LVKW_CURSOR_SHAPE_NWSE_RESIZE:
      idc_name = IDC_SIZENWSE;
      break;
    default:
      idc_name = IDC_ARROW;
      break;
  }

  window->cursor_shape = shape;
  window->current_hcursor = LoadCursorW(NULL, idc_name);

  // Trigger an immediate update if the cursor is over the window
  if (window->cursor_in_client_area) {
    POINT pt;
    GetCursorPos(&pt);
    SetCursorPos(pt.x, pt.y);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_Win32(LVKW_Window *window_handle) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)window_handle;

  SetForegroundWindow(window->hwnd);
  SetFocus(window->hwnd);

  return LVKW_SUCCESS;
}