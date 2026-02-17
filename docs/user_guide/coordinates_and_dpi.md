# Coordinate Systems & High DPI

Modern operating systems, particularly on laptops and mobile devices, use display scaling (also known as High DPI or Retina displays) to improve text readability and interface sharpness.

LVKW handles this complexity by providing distinct coordinate systems. Mixing these up is the most common cause of blurry rendering or incorrectly sized UI elements.

## Definitions

### 1. Logical Units (`LVKW_LogicalVec`, `LVKW_LogicalRect`)
*   **What they represent:** Points in the OS's virtual coordinate space.
*   **Use Cases:**
    *   **Window Sizing:** `createWindow` takes `logical_size`.
    *   **UI Layout:** CSS-like pixels, buttons, text fields.
    *   **Mouse Position:** `MouseMotionEvent.position` is in logical units.
*   **Relationship to Screen:** On a standard monitor (scale 1.0), 1 logical unit = 1 physical pixel. On a HiDPI monitor (scale 2.0), 1 logical unit = 2x2 physical pixels.

### 2. Pixel Units (`LVKW_PixelVec`)
*   **What they represent:** Actual physical pixels on the display panel.
*   **Use Cases:**
    *   **Rendering:**creating swapchains (`vkCreateSwapchainKHR`), allocating framebuffers, setting the viewport.
    *   **Texture Sampling:** Mapping 1:1 to screen pixels.
*   **Access:** Primarily via `LVKW_WindowGeometry` (returned by `getGeometry` or `WindowResizedEvent`).

## The Golden Rule

> **Render to Pixels, Position to Logical.**

1.  **Rendering:** Always use `LVKW_WindowGeometry.pixel_size` to set up your rendering backend (Vulkan swapchain size, OpenGL viewport).
2.  **Input/UI:** Always use `LVKW_WindowGeometry.logical_size` or event positions for hit-testing buttons, moving windows, or calculating layout.

## Handling DPI Changes

When a user moves a window between monitors with different scaling factors (e.g., from a 4K laptop screen to a 1080p external monitor), the OS will trigger a `LVKW_WindowResizedEvent`.

**Correct Handling:**
1.  **Listen:** Catch `LVKW_EVENT_TYPE_WINDOW_RESIZED`.
2.  **Update Rendering:** Re-create or resize your swapchain using `event.resized.geometry.pixel_size`.
3.  **Update UI:** Recalculate layout using `event.resized.geometry.logical_size`.

**Note:** The logical size might remain the same while the pixel size changes drastically, or vice-versa, depending on the OS behavior. Always treat them as independent values provided by the event.
