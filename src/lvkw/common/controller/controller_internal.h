// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CONTROLLER_INTERNAL_H_INCLUDED
#define LVKW_CONTROLLER_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "internal.h"
#include "types_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER

/* Hooks for context.c */
void _lvkw_ctrl_poll(LVKW_Context_Base *ctx_base);
void _lvkw_ctrl_init_context(LVKW_Context_Base *ctx_base);
void _lvkw_ctrl_cleanup_context(LVKW_Context_Base *ctx_base);

#endif /* LVKW_ENABLE_CONTROLLER */

#endif /* LVKW_CONTROLLER_INTERNAL_H_INCLUDED */
