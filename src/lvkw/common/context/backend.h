// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_BACKEND_H_INCLUDED
#define LVKW_BACKEND_H_INCLUDED

#include "types_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    __typeof__(lvkw_context_destroy) *destroy;
    __typeof__(lvkw_display_listVkExtensions) *get_vulkan_instance_extensions;
    __typeof__(lvkw_events_pump) *pump_events;
    __typeof__(lvkw_events_commit) *commit_events;
    __typeof__(lvkw_events_post) *post_event;
    __typeof__(lvkw_events_scan) *scan_events;
    __typeof__(lvkw_context_update) *update;
    __typeof__(lvkw_display_listMonitors) *get_monitors;
    __typeof__(lvkw_display_listMonitorModes) *get_monitor_modes;
    __typeof__(lvkw_instrumentation_getMetrics) *get_metrics;
  } context;

  struct {
    __typeof__(lvkw_display_createWindow) *create;
    __typeof__(lvkw_display_updateWindow) *update;
    __typeof__(lvkw_display_destroyWindow) *destroy;
    __typeof__(lvkw_display_createVkSurface) *create_vk_surface;
    __typeof__(lvkw_display_getWindowGeometry) *get_geometry;
    __typeof__(lvkw_display_requestWindowFocus) *request_focus;
    __typeof__(lvkw_data_pushText) *push_text;
    __typeof__(lvkw_data_pullText) *pull_text;
    __typeof__(lvkw_data_pushData) *push_data;
    __typeof__(lvkw_data_pullData) *pull_data;
    __typeof__(lvkw_data_listBufferMimeTypes) *list_buffer_mime_types;
    __typeof__(lvkw_data_pullTextAsync) *pull_text_async;
    __typeof__(lvkw_data_pullDataAsync) *pull_data_async;
  } window;

  struct {
    __typeof__(lvkw_display_getStandardCursor) *get_standard;
    __typeof__(lvkw_display_createCursor) *create;
    __typeof__(lvkw_display_destroyCursor) *destroy;
  } cursor;

#ifdef LVKW_ENABLE_CONTROLLER
  struct {
    __typeof__(lvkw_input_createController) *create;
    __typeof__(lvkw_input_destroyController) *destroy;
    __typeof__(lvkw_input_getControllerInfo) *getInfo;
    __typeof__(lvkw_input_setControllerHapticLevels) *setHapticLevels;
    __typeof__(lvkw_input_listControllers) *list;
  } ctrl;
#endif
} LVKW_Backend;
#endif

#endif  // LVKW_BACKEND_H_INCLUDED
