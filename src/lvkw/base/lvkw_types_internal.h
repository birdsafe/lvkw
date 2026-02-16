// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TYPES_INTERNAL_H_INCLUDED
#define LVKW_TYPES_INTERNAL_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "lvkw/lvkw.h"
#include "lvkw_string_cache.h"
#include "lvkw_transient_pool.h"
#include "lvkw_thread_internal.h"

#ifdef __cplusplus
#include <atomic>
#define LVKW_ATOMIC(t) std::atomic<t>
#else
#include <stdatomic.h>
#define LVKW_ATOMIC(t) _Atomic t
#endif

typedef struct LVKW_QueueBuffer {
  void *data;
  LVKW_EventType *types;
  LVKW_Window **windows;
  LVKW_Event *payloads;
  uint32_t count;
  uint32_t capacity;

  LVKW_TransientPool transient_pool;
} LVKW_QueueBuffer;

typedef struct LVKW_ExternalEvent {
  LVKW_EventType type;
  LVKW_Window *window;
  LVKW_Event payload;
} LVKW_ExternalEvent;

typedef struct LVKW_EventQueue {
  struct LVKW_Context_Base *ctx;

  LVKW_QueueBuffer buffers[2];
  LVKW_QueueBuffer *active;
  LVKW_QueueBuffer *stable;

  /* Secondary channel for cross-thread events */
  LVKW_ExternalEvent *external;
  uint32_t external_capacity;
  LVKW_ATOMIC(uint32_t) external_head;
  LVKW_ATOMIC(uint32_t) external_tail;
  LVKW_ATOMIC(uint32_t) external_reserve_tail;

  uint32_t max_capacity;
  double growth_factor;

#ifdef LVKW_GATHER_METRICS
  uint32_t peak_count;
  uint32_t drop_count;
#endif
} LVKW_EventQueue;

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
    uint32_t creation_flags;
    LVKW_EventType event_mask;
    LVKW_EventQueue event_queue;
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
