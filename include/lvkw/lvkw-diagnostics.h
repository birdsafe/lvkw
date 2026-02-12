#ifndef LVKW_DIAGNOSTICS_H_INCLUDED
#define LVKW_DIAGNOSTICS_H_INCLUDED

#include <stdint.h>

#include "lvkw-core.h"

/**
 * @file lvkw-diagnostics.h
 * @brief Error reporting and library diagnostics.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Categorization of library-side errors and warnings.
 */
typedef enum LVKW_Diagnostic {
  LVKW_DIAGNOSTIC_NONE = 0,
  LVKW_DIAGNOSTIC_OUT_OF_MEMORY,         ///< The system or custom allocator failed to provide memory.
  LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,  ///< OS-level resource could not be obtained.
  LVKW_DIAGNOSTIC_DYNAMIC_LIB_FAILURE,   ///< Failed to load a required system library
  LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,   ///< Soemthing is not available on the current backend or system.
  LVKW_DIAGNOSTIC_BACKEND_FAILURE,       ///< Something went wrong in the underbelly of the library.
  LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,   ///< The requested backend cannot be created on this system.
  LVKW_DIAGNOSTIC_VULKAN_FAILURE,        ///< Something vulkan-related went wrong.
  LVKW_DIAGNOSTIC_UNKNOWN,               ///< An error occurred that does not fit into other categories.

  /* Debug Diagnostics (Generally unrecoverable; may abort in Debug builds) */

  LVKW_DIAGNOSTIC_INVALID_ARGUMENT,      ///< An argument provided to a public API call was invalid.
  LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,  ///< API state violation (e.g., calling a method from the wrong thread).
  LVKW_DIAGNOSTIC_INTERNAL,              ///< An internal logic error or inconsistent state was detected within LVKW.
} LVKW_Diagnostic;

/**
 * @brief Detailed information about a diagnostic event.
 */
typedef struct LVKW_DiagnosticInfo {
  LVKW_Diagnostic diagnostic;
  const char *message;
  LVKW_Context *context;  ///< NULL if not applicable.
  LVKW_Window *window;    ///< NULL if not applicable.
} LVKW_DiagnosticInfo;

/**
 * @brief Application-defined callback for receiving library diagnostics.
 * @param info Detailed information about the diagnostic event. Do NOT store this pointer; its content is only valid
 * during the call.
 * @param userdata The user-defined pointer provided when the callback was registered.
 */
typedef void (*LVKW_DiagnosticCallback)(const LVKW_DiagnosticInfo *info, void *userdata);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_DIAGNOSTICS_H_INCLUDED
