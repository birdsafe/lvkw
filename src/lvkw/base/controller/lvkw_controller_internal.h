#ifndef LVKW_CONTROLLER_INTERNAL_H_INCLUDED
#define LVKW_CONTROLLER_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw_types_internal.h"

#ifdef LVKW_CONTROLLER_ENABLED

/* Hooks for lvkw_base.c */
void _lvkw_ctrl_poll(LVKW_Context_Base *ctx_base);
void _lvkw_ctrl_init_context(LVKW_Context_Base *ctx_base);
void _lvkw_ctrl_cleanup_context(LVKW_Context_Base *ctx_base);

#endif /* LVKW_CONTROLLER_ENABLED */

#endif /* LVKW_CONTROLLER_INTERNAL_H_INCLUDED */
