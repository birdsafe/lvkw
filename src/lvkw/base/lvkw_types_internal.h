// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TYPES_INTERNAL_H_INCLUDED
#define LVKW_TYPES_INTERNAL_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "lvkw/lvkw.h"
#include "lvkw_string_cache.h"
#include "lvkw_thread_internal.h"

// Forward declaration of LVKW_Backend to allow use in Context/Window
struct LVKW_Backend;

typedef struct LVKW_External_Lib_Base {
  void *handle;
  bool available;
} LVKW_External_Lib_Base;

typedef struct LVKW_Context_Base {
  LVKW_Context pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Allocator alloc_cb;
    void *allocator_userdata;
    LVKW_DiagnosticCallback diagnostic_cb;
    void *diagnostic_userdata;
    struct LVKW_Window_Base *window_list;
#ifdef LVKW_ENABLE_CONTROLLER
    struct LVKW_Controller_Base *controller_list;
#endif
    struct LVKW_Monitor_Base *monitor_list;
    LVKW_StringCache string_cache;
    LVKW_VkGetInstanceProcAddrFunc vk_loader;
    uint32_t creation_flags;
#if LVKW_API_VALIDATION > 0
    LVKW_ThreadId creator_thread;
#endif
  } prv;
} LVKW_Context_Base;

typedef struct LVKW_Monitor_Base {
  LVKW_Monitor pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
    struct LVKW_Monitor_Base *next;
  } prv;
} LVKW_Monitor_Base;

typedef struct LVKW_Window_Base {
  LVKW_Window pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
    struct LVKW_Window_Base *next;

    /* DnD Session state */
    void *session_userdata;
    LVKW_DndAction current_action;
  } prv;
} LVKW_Window_Base;

typedef struct LVKW_Cursor_Base {
  LVKW_Cursor pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
  } prv;
} LVKW_Cursor_Base;

#ifdef LVKW_ENABLE_CONTROLLER
typedef struct LVKW_Controller_Base {
  LVKW_Controller pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
    LVKW_CtrlId id;
    struct LVKW_Controller_Base *next;

    LVKW_AnalogInputState *analogs_backing;
    LVKW_ButtonState *buttons_backing;
    LVKW_AnalogChannelInfo *analog_channels_backing;
    LVKW_ButtonChannelInfo *button_channels_backing;
    LVKW_HapticChannelInfo *haptic_channels_backing;
  } prv;
} LVKW_Controller_Base;
#endif

/* Shared internal helpers */
void _lvkw_context_init_base(LVKW_Context_Base *ctx_base,
                             const LVKW_ContextCreateInfo *create_info);
void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base);

void _lvkw_context_mark_lost(LVKW_Context_Base *ctx_base);
void _lvkw_window_list_add(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);
void _lvkw_window_list_remove(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);

#endif  // LVKW_TYPES_INTERNAL_H_INCLUDED
