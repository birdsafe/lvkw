#include <stdlib.h>
#include <string.h>

#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

static void _lvkw_win32_enable_dpi_awareness(void) {
  // Try SetProcessDpiAwarenessContext (Win 10 1703+)
  HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32) {
    typedef BOOL(WINAPI * SetProcessDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);
    SetProcessDpiAwarenessContext_t set_context =
        (SetProcessDpiAwarenessContext_t)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
    if (set_context) {
      if (set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) return;
    }
  }

  // Try SetProcessDpiAwareness (Win 8.1+)
  HMODULE shcore = LoadLibraryW(L"shcore.dll");
  if (shcore) {
    typedef enum { PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
    typedef HRESULT(WINAPI * SetProcessDpiAwareness_t)(PROCESS_DPI_AWARENESS);
    SetProcessDpiAwareness_t set_awareness = (SetProcessDpiAwareness_t)GetProcAddress(shcore, "SetProcessDpiAwareness");
    if (set_awareness) {
      if (set_awareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK) return;
    }
  }

  // Fallback to SetProcessDPIAware (Vista+)
  SetProcessDPIAware();
}

LVKW_Status lvkw_ctx_create_Win32(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  _lvkw_win32_enable_dpi_awareness();

  LVKW_Allocator allocator = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};
  if (create_info->allocator.alloc_cb) {
    allocator = create_info->allocator;
  }

  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)lvkw_alloc(&allocator, create_info->userdata, sizeof(LVKW_Context_Win32));
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_OUT_OF_MEMORY,
                                    "Failed to allocate storage for context");
    return LVKW_ERROR;
  }

  _lvkw_context_init_base(&ctx->base, create_info);
  ctx->base.prv.alloc_cb = allocator;
  ctx->hInstance = GetModuleHandle(NULL);
  ctx->last_event_time = GetTickCount();

  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = _lvkw_win32_wndproc;
  wc.hInstance = ctx->hInstance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = L"LVKW_WindowClass";

  ctx->window_class_atom = RegisterClassExW(&wc);
  if (!ctx->window_class_atom) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "Failed to register window class");
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  // Apply initial attributes
  lvkw_ctx_update_Win32((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;
}

void lvkw_ctx_destroy_Win32(LVKW_Context *ctx_handle) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;

  // Destroy all windows in list
  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_Win32((LVKW_Window *)ctx->base.prv.window_list);
  }

  UnregisterClassW((LPCWSTR)(uintptr_t)ctx->window_class_atom, ctx->hInstance);
  lvkw_context_free(&ctx->base, ctx);
}

void lvkw_ctx_getVkExtensions_Win32(LVKW_Context *ctx_handle, uint32_t *count,
                                                    const char **out_extensions) {
  static const char *extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_win32_surface",
  };
  uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

  if (out_extensions == NULL) {
    *count = extension_count;
    return;
  }

  uint32_t to_copy = (*count < extension_count) ? *count : extension_count;
  for (uint32_t i = 0; i < to_copy; ++i) {
    out_extensions[i] = extensions[i];
  }
  *count = to_copy;
}

LVKW_Status lvkw_ctx_update_Win32(LVKW_Context *ctx_handle, uint32_t field_mask,
                                                 const LVKW_ContextAttributes *attributes) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    if (ctx->inhibit_idle != attributes->inhibit_idle) {
      if (attributes->inhibit_idle) {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
      }
      else {
        SetThreadExecutionState(ES_CONTINUOUS);
      }
      ctx->inhibit_idle = attributes->inhibit_idle;
    }
  }

  return LVKW_SUCCESS;
}