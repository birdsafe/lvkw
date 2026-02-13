// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_string_cache.h"

#include <string.h>

#include "lvkw_mem_internal.h"
#include "lvkw_types_internal.h"

static uint32_t _fnv1a(const char *str) {
  uint32_t hash = 2166136261u;
  for (const char *p = str; *p; p++) {
    hash ^= (uint32_t)(unsigned char)*p;
    hash *= 16777619u;
  }
  return hash;
}

void _lvkw_string_cache_init(LVKW_StringCache *cache) {
  memset(cache, 0, sizeof(*cache));
}

void _lvkw_string_cache_destroy(LVKW_StringCache *cache, LVKW_Context_Base *ctx_base) {
  for (uint32_t i = 0; i < cache->count; i++) {
    lvkw_context_free(ctx_base, cache->entries[i].str);
  }
  cache->count = 0;
}

const char *_lvkw_string_cache_intern(LVKW_StringCache *cache, LVKW_Context_Base *ctx_base, const char *str) {
  if (!str) return NULL;

  uint32_t hash = _fnv1a(str);

  /* Linear scan for existing entry */
  for (uint32_t i = 0; i < cache->count; i++) {
    if (cache->entries[i].hash == hash && strcmp(cache->entries[i].str, str) == 0) {
      return cache->entries[i].str;
    }
  }

  /* Not found — intern it */
  if (cache->count >= LVKW_STRING_CACHE_MAX_ENTRIES) {
    return NULL;  /* Cache full */
  }

  size_t len = strlen(str);
  char *copy = (char *)lvkw_context_alloc(ctx_base, len + 1);
  if (!copy) return NULL;

  memcpy(copy, str, len + 1);

  cache->entries[cache->count].hash = hash;
  cache->entries[cache->count].str = copy;
  cache->count++;

  return copy;
}
