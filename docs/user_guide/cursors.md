# Custom Cursors

## Standard System Cursors

The OS provides a set of standard cursors (Arrow, Hand, I-beam, etc.). These are managed by the LVKW context and are always available.

```cpp
// Get a standard pointer (hand) cursor
LVKW_Cursor* hand = ctx.getStandardCursor(LVKW_CURSOR_SHAPE_POINTER);

// Apply it to a window
window.setCursor(hand);
```

**Ownership:** You do **not** own system cursors. Never attempt to destroy a cursor handle retrieved via `getStandardCursor`.

## Custom Cursors

You can create a cursor from raw RGBA8888 pixel data.

### 1. Define the Cursor
```cpp
// 32x32 transparent red square with a hotspot in the middle
uint32_t pixels[32 * 32];
std::fill_n(pixels, 32 * 32, 0xFFFF0000); 

LVKW_CursorCreateInfo ci = {};
ci.size = {32, 32};
ci.hotSpot = {16, 16}; // Interaction point relative to top-left
ci.pixels = pixels;

lvkw::Cursor my_cursor = ctx.createCursor(ci);
```

These are subject to OS size limits (typically 32x32 or 64x64). If you provide a larger image, the OS may downscale or crop it.


## Cursor Visibility Modes

You can control the visibility and constraint of the cursor via `LVKW_CursorMode`.

| Mode | Behavior |
| :--- | :--- |
| `LVKW_CURSOR_NORMAL` | Visible and free to move. |
| `LVKW_CURSOR_HIDDEN` | Invisible when over the window, but can still leave it. |
| `LVKW_CURSOR_LOCKED` | Invisible and confined to the window center. Delivers raw relative motion (ideal for 3D cameras). |

```cpp
window.setCursorMode(LVKW_CURSOR_LOCKED);
```