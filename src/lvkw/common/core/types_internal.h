// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TYPES_INTERNAL_H_INCLUDED
#define LVKW_TYPES_INTERNAL_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "lvkw/lvkw.h"
#include "string_cache.h"
#include "transient_pool.h"
#include "thread_internal.h"

#ifdef __cplusplus
#include <atomic>
#define LVKW_ATOMIC(t) std::atomic<t>
#else
#include <stdatomic.h>
#define LVKW_ATOMIC(t) _Atomic t
#endif

typedef struct LVKW_ExternalEvent {
  LVKW_EventType type;
  LVKW_Window *window;
  LVKW_Event payload;
} LVKW_ExternalEvent;

typedef struct LVKW_EventNotificationRing {
  LVKW_ExternalEvent *buffer;
  uint32_t capacity;
  LVKW_ATOMIC(uint32_t) head;
  LVKW_ATOMIC(uint32_t) tail;
  LVKW_ATOMIC(uint32_t) reserve_tail;
} LVKW_EventNotificationRing;

// Forward declaration of LVKW_Backend to allow use in Context/Window
struct LVKW_Backend;

typedef struct LVKW_External_Lib_Base {
  void *handle;
  bool available;
} LVKW_External_Lib_Base;

typedef struct LVKW_Cursor_Base {
  LVKW_Cursor pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Context_Base *ctx_base;
    LVKW_CursorShape shape;
    uintptr_t backend_data[2];
  } prv;
} LVKW_Cursor_Base;

typedef struct LVKW_Context_Base {
  LVKW_Context pub;

  struct {
#ifdef LVKW_INDIRECT_BACKEND
    const struct LVKW_Backend *backend;
#endif
    LVKW_Allocator allocator;
    LVKW_DiagnosticCallback diagnostic_cb;
    void *diagnostic_userdata;
    struct LVKW_Window_Base *window_list;
#ifdef LVKW_ENABLE_CONTROLLER
    struct LVKW_Controller_Base *controller_list;
#endif
    struct LVKW_Monitor_Base *monitor_list;
    LVKW_StringCache string_cache;
    LVKW_VkGetInstanceProcAddrFunc vk_loader;
    LVKW_Cursor_Base standard_cursors[13]; // 1..12
    LVKW_ATOMIC(uint32_t) event_mask;
    uint32_t pump_event_mask;
    
    LVKW_EventCallback event_callback;
    void *event_userdata;

    LVKW_EventNotificationRing external_notifications;
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
    uint32_t user_refcount;
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
    LVKW_DndFeedback dnd_feedback;
  } prv;
} LVKW_Window_Base;



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
    uint32_t user_refcount;

    LVKW_AnalogInputState *analogs_backing;
    LVKW_ButtonState *buttons_backing;
    LVKW_AnalogChannelInfo *analog_channels_backing;
    LVKW_ButtonChannelInfo *button_channels_backing;
    LVKW_HapticChannelInfo *haptic_channels_backing;
  } prv;
} LVKW_Controller_Base;
#endif

/* Shared internal helpers */
LVKW_Status _lvkw_context_init_base(LVKW_Context_Base *ctx_base,
                                    const LVKW_ContextCreateInfo *create_info);
void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base);

void _lvkw_update_base_attributes(LVKW_Context_Base *ctx_base, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes);

void _lvkw_context_mark_lost(LVKW_Context_Base *ctx_base);
void _lvkw_window_list_add(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);
void _lvkw_window_list_remove(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base);

#endif  // LVKW_TYPES_INTERNAL_H_INCLUDED
