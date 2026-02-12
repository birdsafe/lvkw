#ifndef LVKW_INTERNAL_H_INCLUDED
#define LVKW_INTERNAL_H_INCLUDED

/* 
 * This is the umbrella header for all internal LVKW definitions.
 * It is split into specialized headers to improve maintainability.
 */

#include "lvkw_types_internal.h"
#include "lvkw_backend.h"
#include "lvkw_diagnostic_internal.h"
#include "lvkw_mem_internal.h"

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);

#endif // LVKW_INTERNAL_H_INCLUDED