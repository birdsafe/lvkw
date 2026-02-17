// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LIBRARY_H_INCLUDED
#define LVKW_LIBRARY_H_INCLUDED

#include "lvkw/details/lvkw_config.h"

#include "lvkw/c/context.h"
#include "lvkw/c/core.h"
#include "lvkw/c/data.h"
#include "lvkw/c/display.h"
#include "lvkw/c/events.h"
#include "lvkw/c/input.h"
#include "lvkw/c/instrumentation.h"
#include "lvkw/c/shortcuts.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "lvkw/c/ext/controller.h"
#endif

#include "lvkw/details/lvkw_abi_checks.h"

#endif  // LVKW_LIBRARY_H_INCLUDED
