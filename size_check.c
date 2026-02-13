#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define LVKW_USE_FLOAT
#include "lvkw/lvkw-events.h"

int main() {
    printf("sizeof(LVKW_Event) = %zu\n", sizeof(LVKW_Event));
    printf("sizeof(LVKW_MouseMotionEvent) = %zu\n", sizeof(LVKW_MouseMotionEvent));
    printf("sizeof(LVKW_DndHoverEvent) = %zu\n", sizeof(LVKW_DndHoverEvent));
    printf("sizeof(LVKW_DndDropEvent) = %zu\n", sizeof(LVKW_DndDropEvent));
    printf("sizeof(LVKW_TextCompositionEvent) = %zu\n", sizeof(LVKW_TextCompositionEvent));
    return 0;
}
