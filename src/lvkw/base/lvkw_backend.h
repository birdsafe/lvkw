// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_BACKEND_H_INCLUDED
#define LVKW_BACKEND_H_INCLUDED

#include "lvkw_types_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    typeof(lvkw_ctx_destroy) *destroy;
    typeof(lvkw_ctx_getVkExtensions) *get_vulkan_instance_extensions;
    typeof(lvkw_ctx_pollEvents) *poll_events;
    typeof(lvkw_ctx_waitEvents) *wait_events;
    typeof(lvkw_ctx_update) *update;
    typeof(lvkw_ctx_getMonitors) *get_monitors;
    typeof(lvkw_ctx_getMonitorModes) *get_monitor_modes;
  } context;

  struct {
    typeof(lvkw_ctx_createWindow) *create;
    typeof(lvkw_wnd_update) *update;
    typeof(lvkw_wnd_destroy) *destroy;
    typeof(lvkw_wnd_createVkSurface) *create_vk_surface;
    typeof(lvkw_wnd_getGeometry) *get_geometry;
    typeof(lvkw_wnd_requestFocus) *request_focus;
    typeof(lvkw_wnd_setClipboardText) *set_clipboard_text;
    typeof(lvkw_wnd_getClipboardText) *get_clipboard_text;
    typeof(lvkw_wnd_setClipboardData) *set_clipboard_data;
    typeof(lvkw_wnd_getClipboardData) *get_clipboard_data;
    typeof(lvkw_wnd_getClipboardMimeTypes) *get_clipboard_mime_types;
  } window;

  struct {
    typeof(lvkw_ctx_getStandardCursor) *get_standard;
    typeof(lvkw_ctx_createCursor) *create;
    typeof(lvkw_cursor_destroy) *destroy;
  } cursor;

#ifdef LVKW_ENABLE_CONTROLLER
  struct {
    typeof(lvkw_ctrl_create) *create;
    typeof(lvkw_ctrl_destroy) *destroy;
    typeof(lvkw_ctrl_getInfo) *getInfo;
    typeof(lvkw_ctrl_setHapticLevels) *setHapticLevels;
  } ctrl;
#endif
} LVKW_Backend;
#endif

#endif  // LVKW_BACKEND_H_INCLUDED
