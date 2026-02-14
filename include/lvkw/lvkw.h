// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LIBRARY_H_INCLUDED
#define LVKW_LIBRARY_H_INCLUDED

#include "lvkw/details/lvkw_config.h"

// ...

#include "lvkw-context.h"
#include "lvkw-core.h"
#include "lvkw-cursor.h"
#include "lvkw-diagnostics.h"
#include "lvkw-events.h"
#include "lvkw-input.h"
#include "lvkw-monitor.h"
#include "lvkw-telemetry.h"
#include "lvkw-tuning.h"
#include "lvkw-window.h"
#include "lvkw-shortcuts.h"
#include "lvkw/details/lvkw_details.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "lvkw-ext-controller.h"
#endif

// ...

#include "lvkw/details/lvkw_abi_checks.h"

#endif
