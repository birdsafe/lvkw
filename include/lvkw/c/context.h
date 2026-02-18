// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CONTEXT_H_INCLUDED
#define LVKW_CONTEXT_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw/c/core.h"
#include "lvkw/details/lvkw_details.h"

/**
 * @file context.h
 * @brief Context management, lifecycle attributes, and low-level tuning.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct LVKW_DiagnosticInfo;
typedef void (*LVKW_DiagnosticCallback)(const struct LVKW_DiagnosticInfo *info, void *userdata);

/** @brief Runtime status flags for a context. */
typedef enum LVKW_ContextFlags {
  LVKW_CONTEXT_STATE_LOST = 1 << 0,   ///< Every operation on that context will fail.
                                  ///< It should be destroyed.
  LVKW_CONTEXT_STATE_READY = 1 << 1,  ///< Context is fully initialized and ready for use. This is
                                  ///< guaranteed to be set if lvkw_context_create() returns
                                  ///< LVKW_SUCCESS. Its only use is in the middle of context
                                  ///< creation.
} LVKW_ContextFlags;

/**
 * @brief Opaque handle representing the library state and display server
 * connection.
 *
 * ### Threading At A Glance
 * - The thread that calls @ref lvkw_context_create becomes the context's
 *   primary thread.
 * - APIs documented as primary-thread-only MUST run on that thread.
 * - A small subset of read/event APIs are callable from any thread, but still
 *   require explicit external synchronization as documented per function.
 */
struct LVKW_Context {
  void *userdata;  ///< User-controlled pointer. You CAN override it directly.
  LVKW_READONLY uint32_t flags;  ///< Bitmask of LVKW_ContextFlags.
};

/* ----- Context Management ----- */

/** @brief Special value for timeouts to indicate it should never trigger. */
#define LVKW_NEVER (uint32_t)-1

/** @brief Flags for context creation. */
typedef enum LVKW_ContextCreationFlags {
  LVKW_CONTEXT_FLAG_NONE = 0,
} LVKW_ContextCreationFlags;

/** @brief Bitmask for selecting which attributes to update in
 * lvkw_context_update(). */
typedef enum LVKW_ContextAttributesField {
  LVKW_CONTEXT_ATTR_INHIBIT_IDLE = 1 << 0,   ///< Update inhibit_idle.
  LVKW_CONTEXT_ATTR_DIAGNOSTICS = 1 << 1,    ///< Update diagnostic_cb and diagnostic_userdata.
  LVKW_CONTEXT_ATTR_EVENT_MASK = 1 << 2,     ///< Update event_mask.
  LVKW_CONTEXT_ATTR_EVENT_CALLBACK = 1 << 3, ///< Update event_callback and event_userdata.

  LVKW_CONTEXT_ATTR_ALL = LVKW_CONTEXT_ATTR_INHIBIT_IDLE | LVKW_CONTEXT_ATTR_DIAGNOSTICS |
                          LVKW_CONTEXT_ATTR_EVENT_MASK | LVKW_CONTEXT_ATTR_EVENT_CALLBACK,
} LVKW_ContextAttributesField;

/** @brief Configurable parameters for an active context. */
typedef struct LVKW_ContextAttributes {
  bool inhibit_idle;                      ///< If true, prevents the OS from entering
                                          ///< sleep/power-save modes.
  LVKW_DiagnosticCallback diagnostic_cb;  ///< Optional callback for library diagnostics.
  void *diagnostic_userdata;              ///< Passed to the diagnostic callback.
  LVKW_EventType event_mask;              ///< Bitmask of LVKW_EventType to allow.
  LVKW_EventCallback event_callback;      ///< Primary event sink.
  void *event_userdata;                   ///< Passed to the event callback.
} LVKW_ContextAttributes;

/** @brief Supported windowing backends. */
typedef enum LVKW_BackendType {
  LVKW_BACKEND_AUTO = 0,     ///< Select best available backend for the current platform.
  LVKW_BACKEND_WAYLAND = 1,  ///< Linux Wayland.
  LVKW_BACKEND_X11 = 2,      ///< Linux X11.
  LVKW_BACKEND_WINDOWS = 3,  ///< Windows.
  LVKW_BACKEND_COCOA = 4,    ///< MacOS Cocoa.
} LVKW_BackendType;

/** @brief Window decoration strategies for Wayland backends. */
typedef enum LVKW_WaylandDecorationMode {
  LVKW_WAYLAND_DECORATION_MODE_AUTO = 0,  ///< Try to use SSD, falls back to CSD
  LVKW_WAYLAND_DECORATION_MODE_SSD = 1,   ///< Force Server-Side Decorations
  LVKW_WAYLAND_DECORATION_MODE_CSD = 2,   ///< Force Client-Side Decorations via libdecor.
  LVKW_WAYLAND_DECORATION_MODE_NONE = 3,  ///< Disable all decorations.
} LVKW_WaylandDecorationMode;

/** @brief Generic function pointer for Vulkan return types. */
typedef void (*LVKW_VulkanVoidFunction)(void);

/** @brief Function pointer signature for vkGetInstanceProcAddr. */
typedef LVKW_VulkanVoidFunction (*LVKW_VkGetInstanceProcAddrFunc)(VkInstance instance,
                                                                  const char *pName);

/** @brief Backend-specific and internal library tuning.
 *  @note Most applications should use LVKW_CONTEXT_TUNING_DEFAULT. */
typedef struct LVKW_ContextTuning {
  struct {
    LVKW_WaylandDecorationMode decoration_mode;
    /**
     * @brief How long to wait after a dnd drop before giving up on receiving the payload.  
     */
    uint32_t dnd_post_drop_timeout_ms;
    /**
     * @brief If true, apply client-side constraint fixes (like aspect ratio) when the
     * compositor does not enforce them directly.
     */
    bool enforce_client_side_constraints;
  } wayland;

  struct {
    /**
     * @brief Polling period for X11 system-idle detection, in milliseconds.
     *
     * Used only by the X11 backend when querying XScreenSaver idle state.
     * Set to 0 to use backend default.
     */
    uint32_t idle_poll_interval_ms;
  } x11;

  /**
   * @brief Optional override for the Vulkan loader entry point.
   *
   * If NULL (default), LVKW will use the symbol `vkGetInstanceProcAddr` from
   * the linked Vulkan library. If provided, LVKW will use this function pointer
   * instead. This is useful for custom loaders or when the Vulkan library is
   * not directly linked.
   */
  LVKW_VkGetInstanceProcAddrFunc vk_loader;
} LVKW_ContextTuning;

/**
 * @brief Default tuning parameters optimized for general-purpose applications.
 */
#define LVKW_CONTEXT_TUNING_DEFAULT                                                      \
  {                                                                                      \
   .wayland =                                                                            \
       {.decoration_mode = LVKW_WAYLAND_DECORATION_MODE_AUTO, .dnd_post_drop_timeout_ms = 1000, \
        .enforce_client_side_constraints = true},                                         \
   .x11 = {.idle_poll_interval_ms = 250},                                                 \
   .vk_loader = NULL}

/** @brief Parameters for lvkw_context_create(). */
typedef struct LVKW_ContextCreateInfo {
  LVKW_Allocator allocator;           ///< Custom memory allocator. Set to zero for defaults.
  void *userdata;                     ///< Initial value for LVKW_Context::userdata.
  LVKW_BackendType backend;           ///< Explicitly request a backend or use LVKW_BACKEND_AUTO.
  uint32_t flags;                     ///< Bitmask of LVKW_ContextCreationFlags.
  LVKW_ContextAttributes attributes;  ///< Initial runtime attributes.
  const LVKW_ContextTuning *tuning;   ///< Optional low-level minutia.
} LVKW_ContextCreateInfo;

/**
 * @brief Default initialization macro for LVKW_ContextCreateInfo.
 * @note Use it like this: LVKW_ContextCreateInfo cci =
 * LVKW_CONTEXT_CREATE_INFO_DEFAULT;
 */
#define LVKW_CONTEXT_CREATE_INFO_DEFAULT      \
  {                                           \
      .backend = LVKW_BACKEND_AUTO,           \
      .flags = LVKW_CONTEXT_FLAG_NONE,            \
      .attributes =                           \
          {                                   \
              .inhibit_idle = false,          \
              .event_mask = LVKW_EVENT_TYPE_ALL, \
          },                                  \
      .tuning = NULL,                         \
  }

/**
 * @brief Creates a new context.
 * @note The calling thread becomes the context's primary thread.
 * @param create_info Configuration for the new context.
 * @param[out] out_context Receives the pointer to the new context handle.
 */
LVKW_COLD LVKW_Status lvkw_context_create(const LVKW_ContextCreateInfo *create_info,
                                          LVKW_Context **out_context);

/**
 * @brief Destroys a context and all associated windows/resources.
 * @note Must be called on the context's primary thread.
 * @param context The context to destroy.
 */
LVKW_COLD LVKW_Status lvkw_context_destroy(LVKW_Context *context);

/**
 * @brief Updates one or more context attributes.
 * @note Non-flagged values are ignored and can be left uninitialized.
 * @param context Active context.
 * @param field_mask Mask of LVKW_ContextAttributesField indicating which fields
 * to read from @p attributes.
 * @param attributes Source struct containing the new values.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_context_update(LVKW_Context *context, uint32_t field_mask,
                                          const LVKW_ContextAttributes *attributes);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CONTEXT_H_INCLUDED
