// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_BACKEND_H_INCLUDED
#define LVKW_BACKEND_H_INCLUDED

#include "lvkw_types_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    __typeof__(lvkw_ctx_destroy) *destroy;
    __typeof__(lvkw_ctx_getVkExtensions) *get_vulkan_instance_extensions;
    __typeof__(lvkw_ctx_syncEvents) *sync_events;
    __typeof__(lvkw_ctx_postEvent) *post_event;
    __typeof__(lvkw_ctx_scanEvents) *scan_events;
    __typeof__(lvkw_ctx_update) *update;
    __typeof__(lvkw_ctx_getMonitors) *get_monitors;
    __typeof__(lvkw_ctx_getMonitorModes) *get_monitor_modes;
    __typeof__(lvkw_ctx_getTelemetry) *get_telemetry;
  } context;

  struct {
    __typeof__(lvkw_ctx_createWindow) *create;
    __typeof__(lvkw_wnd_update) *update;
    __typeof__(lvkw_wnd_destroy) *destroy;
    __typeof__(lvkw_wnd_createVkSurface) *create_vk_surface;
    __typeof__(lvkw_wnd_getGeometry) *get_geometry;
    __typeof__(lvkw_wnd_requestFocus) *request_focus;
    __typeof__(lvkw_wnd_setClipboardText) *set_clipboard_text;
    __typeof__(lvkw_wnd_getClipboardText) *get_clipboard_text;
    __typeof__(lvkw_wnd_setClipboardData) *set_clipboard_data;
    __typeof__(lvkw_wnd_getClipboardData) *get_clipboard_data;
    __typeof__(lvkw_wnd_getClipboardMimeTypes) *get_clipboard_mime_types;
  } window;

  struct {
    __typeof__(lvkw_ctx_getStandardCursor) *get_standard;
    __typeof__(lvkw_ctx_createCursor) *create;
    __typeof__(lvkw_cursor_destroy) *destroy;
  } cursor;

#ifdef LVKW_ENABLE_CONTROLLER
  struct {
    __typeof__(lvkw_ctrl_create) *create;
    __typeof__(lvkw_ctrl_destroy) *destroy;
    __typeof__(lvkw_ctrl_getInfo) *getInfo;
    __typeof__(lvkw_ctrl_setHapticLevels) *setHapticLevels;
  } ctrl;
#endif
} LVKW_Backend;
#endif

#endif  // LVKW_BACKEND_H_INCLUDED
