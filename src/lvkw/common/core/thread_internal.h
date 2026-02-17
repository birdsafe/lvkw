// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_THREAD_INTERNAL_H_INCLUDED
#define LVKW_THREAD_INTERNAL_H_INCLUDED

#include <stdint.h>

typedef uint64_t LVKW_ThreadId;

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static inline LVKW_ThreadId _lvkw_get_current_thread_id(void) {
  return (LVKW_ThreadId)GetCurrentThreadId();
}
#else
#include <pthread.h>
static inline LVKW_ThreadId _lvkw_get_current_thread_id(void) {
  return (LVKW_ThreadId)pthread_self();
}
#endif

#endif  // LVKW_THREAD_INTERNAL_H_INCLUDED
