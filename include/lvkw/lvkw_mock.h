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
void lvkw_mock_pushEvent(LVKW_Context *ctx, const LVKW_Event *evt);

#ifdef __cplusplus
}
#endif

#endif
