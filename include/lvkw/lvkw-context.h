// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CONTEXT_H_INCLUDED
#define LVKW_CONTEXT_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw-core.h"
#include "lvkw-diagnostics.h"
#include "lvkw-tuning.h"
#include "lvkw/details/lvkw_details.h"

/**
 * @file lvkw-context.h
 * @brief Context management and library initialization.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Runtime status flags for a context. */
typedef enum LVKW_ContextFlags {
  LVKW_CTX_STATE_LOST = 1 << 0,   ///< Every operation on that context will fail.
                                  ///< It should be destroyed.
  LVKW_CTX_STATE_READY = 1 << 1,  ///< Context is fully initialized and ready for use. This is
                                  ///< guaranteed to be set if lvkw_createContext() returns
                                  ///< LVKW_SUCCESS. Its only use is in the middle of context
                                  ///< creation.
} LVKW_ContextFlags;

/**
 * @brief Opaque handle representing the library state and display server
 * connection.
 *
 * ### Threading Model
 * By default, LVKW follows a **Thread-Bound** model where all calls must occur
 * on the thread that created the context.
 *
 * If the @ref LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API flag is provided during
 * creation, the library enters a **Hybrid** model:
 * 1. **Main-Thread Bound:** Creation, destruction, window management
 * (create/destroy), and event polling MUST occur on the creator thread.
 * 2. **Cross-Thread Permissive:** All other functions (attribute updates,
 * geometry queries, haptics) may be called from any thread, provided the user
 * ensures **external synchronization** (e.g., a mutex) so that no two threads
 * enter the LVKW context concurrently.
 */
struct LVKW_Context {
  void *userdata;  ///< User-controlled pointer. You CAN override it directly.
  uint32_t flags;  ///< Bitmask of LVKW_ContextFlags. Read-only.
};

/* ----- Context Management ----- */

/** @brief Special value for timeouts to indicate it should never trigger. */
#define LVKW_NEVER (uint32_t)-1

/** @brief Flags for context creation. */
typedef enum LVKW_ContextCreationFlags {
  LVKW_CTX_FLAG_NONE = 0,
  /**
   * @brief Allows specific API functions to be called from threads other than
   * the one that created the context.
   * @note REQUIRES EXTERNAL SYNCHRONIZATION.
   */
  LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API = 1 << 0,
} LVKW_ContextCreationFlags;

/** @brief Bitmask for selecting which attributes to update in
 * lvkw_ctx_update(). */
typedef enum LVKW_ContextAttributesField {
  LVKW_CTX_ATTR_IDLE_TIMEOUT = 1 << 0,  ///< Update idle_timeout_ms.
  LVKW_CTX_ATTR_INHIBIT_IDLE = 1 << 1,   ///< Update inhibit_idle.
  LVKW_CTX_ATTR_DIAGNOSTICS = 1 << 2,    ///< Update diagnostic_cb and diagnostic_userdata.
  LVKW_CTX_ATTR_EVENT_MASK = 1 << 3,     ///< Update event_mask.

  LVKW_CTX_ATTR_ALL = LVKW_CTX_ATTR_IDLE_TIMEOUT | LVKW_CTX_ATTR_INHIBIT_IDLE |
                      LVKW_CTX_ATTR_DIAGNOSTICS | LVKW_CTX_ATTR_EVENT_MASK,
} LVKW_ContextAttributesField;

/** @brief Configurable parameters for an active context. */
typedef struct LVKW_ContextAttributes {
  uint32_t idle_timeout_ms;               ///< Milliseconds before the system enters idle
                                          ///< state. Use LVKW_NEVER to disable.
  bool inhibit_idle;                      ///< If true, prevents the OS from entering
                                          ///< sleep/power-save modes.
  LVKW_DiagnosticCallback diagnostic_cb;  ///< Optional callback for library diagnostics.
  void *diagnostic_userdata;              ///< Passed to the diagnostic callback.
  LVKW_EventType event_mask;              ///< Bitmask of LVKW_EventType to allow into the
                                          ///< queue.
} LVKW_ContextAttributes;

/** @brief Supported windowing backends. */
typedef enum LVKW_BackendType {
  LVKW_BACKEND_AUTO = 0,     ///< Select best available backend for the current platform.
  LVKW_BACKEND_WAYLAND = 1,  ///< Linux Wayland.
  LVKW_BACKEND_X11 = 2,      ///< Linux X11.
  LVKW_BACKEND_WIN32 = 3,    ///< Windows Win32.
  LVKW_BACKEND_COCOA = 4,    ///< MacOS Cocoa.
} LVKW_BackendType;

/** @brief Parameters for lvkw_createContext(). */
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
      .flags = LVKW_CTX_FLAG_NONE,            \
      .attributes =                           \
          {                                   \
              .idle_timeout_ms = LVKW_NEVER,  \
              .inhibit_idle = false,          \
              .event_mask = LVKW_EVENT_TYPE_ALL, \
          },                                  \
      .tuning = NULL,                         \
  }

/**
 * @brief Creates a new context.
 * @note **Thread Affinity:** The thread that calls this function becomes the
 * "main thread" for that context. All subsequent calls involving this it or its
 * windows must occur on this same thread.
 * @param create_info Configuration for the new context.
 * @param[out] out_context Receives the pointer to the new context handle.
 */
LVKW_COLD LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info,
                                         LVKW_Context **out_context);

/**
 * @brief Destroys a context and all associated windows/resources.
 * @param ctx_handle The context to destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctx_destroy(LVKW_Context *ctx_handle);

/**
 * @brief Returns the list of Vulkan instance extensions required by the
 * selected backend.
 * @note Use the returned strings when filling
 * VkInstanceCreateInfo::ppEnabledExtensionNames.
 * @note The strings and the array are guaranteed to live at least as long as
 * the context. Do NOT free them.
 * @param ctx_handle Active context.
 * @param[out] out_count Receives the number of extension strings returned.
 * @param[out] out_extensions Receives the array of null-terminated UTF-8
 * strings.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *out_count,
                                               const char *const **out_extensions);

/**
 * @brief Updates one or more context attributes.
 * @note Non-flagged values are ignored and can be left uninitialized.
 * @param ctx_handle Active context.
 * @param field_mask Mask of LVKW_ContextAttributesField indicating which fields
 * to read from @p attributes.
 * @param attributes Source struct containing the new values.
 */
LVKW_COLD LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                                      const LVKW_ContextAttributes *attributes);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CONTEXT_H_INCLUDED
