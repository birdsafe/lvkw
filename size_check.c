#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef USE_FLOAT
#define LVKW_USE_FLOAT
#endif

#include "lvkw/lvkw-events.h"

int main() {
#ifdef USE_FLOAT
    printf("--- FLOAT ---\n");
#else
    printf("--- DOUBLE ---\n");
#endif
    printf("sizeof(LVKW_Event) = %zu\n", sizeof(LVKW_Event));
    printf("sizeof(LVKW_MonitorConnectionEvent) = %zu\n", sizeof(LVKW_MonitorConnectionEvent));
    printf("sizeof(LVKW_MonitorModeEvent) = %zu\n", sizeof(LVKW_MonitorModeEvent));
    printf("sizeof(LVKW_MouseMotionEvent) = %zu\n", sizeof(LVKW_MouseMotionEvent));
    printf("sizeof(LVKW_DndHoverEvent) = %zu\n", sizeof(LVKW_DndHoverEvent));
    printf("sizeof(LVKW_DndDropEvent) = %zu\n", sizeof(LVKW_DndDropEvent));
    printf("sizeof(LVKW_TextCompositionEvent) = %zu\n", sizeof(LVKW_TextCompositionEvent));
    return 0;
}
