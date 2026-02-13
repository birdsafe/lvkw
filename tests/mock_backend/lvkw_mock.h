// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_MOCK_H_INCLUDED
#define LVKW_MOCK_H_INCLUDED

#include "lvkw/lvkw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pushes an event into the mock context's event queue.
 * This is intended for testing purposes to simulate OS events.
 */
void lvkw_mock_pushEvent(LVKW_Context *handle, LVKW_EventType type, LVKW_Window* window, const LVKW_Event *evt);
void lvkw_mock_markWindowReady(LVKW_Window *window);

/* Test helpers for injecting monitor state */
LVKW_Monitor* lvkw_mock_addMonitor(LVKW_Context *ctx, const char* name, LVKW_LogicalVec logical_size);
void lvkw_mock_removeMonitor(LVKW_Context *ctx, LVKW_Monitor *monitor);
void lvkw_mock_addMonitorMode(LVKW_Context *ctx, LVKW_Monitor *monitor, LVKW_VideoMode mode);

#ifdef __cplusplus
}
#endif

#endif
