#ifndef LVKW_STRING_CACHE_H_INCLUDED
#define LVKW_STRING_CACHE_H_INCLUDED

#include <stdint.h>

#define LVKW_STRING_CACHE_MAX_ENTRIES 32

typedef struct LVKW_StringCacheEntry {
  uint32_t hash;
  char *str;
} LVKW_StringCacheEntry;

typedef struct LVKW_StringCache {
  LVKW_StringCacheEntry entries[LVKW_STRING_CACHE_MAX_ENTRIES];
  uint32_t count;
} LVKW_StringCache;

/* Forward declaration */
typedef struct LVKW_Context_Base LVKW_Context_Base;

void _lvkw_string_cache_init(LVKW_StringCache *cache);
void _lvkw_string_cache_destroy(LVKW_StringCache *cache, LVKW_Context_Base *ctx_base);

/* Intern a string. Returns a pointer that is stable until cache destruction.
   If the string was already interned, returns the existing pointer. */
const char *_lvkw_string_cache_intern(LVKW_StringCache *cache, LVKW_Context_Base *ctx_base, const char *str);

#endif  // LVKW_STRING_CACHE_H_INCLUDED
