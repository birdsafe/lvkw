# Monitor & Display Management

## Backend Support Status

- Linux (X11, Wayland): monitor enumeration and modes are implemented.
- macOS: monitor APIs currently return an empty result (`count = 0`).
- Win32: backend is currently stubbed.

## Enumerating Monitors

You can retrieve a list of all currently connected monitors from the context.

```cpp
std::vector<LVKW_MonitorRef*> refs = ctx.getMonitors();

for (auto* ref : refs) {
    LVKW_Monitor* monitor = ctx.createMonitor(ref);
    std::cout << "Monitor: " << monitor->name << "\n"
              << "  Primary: " << (monitor->is_primary ? "Yes" : "No") << "\n"
              << "  DPI Scale: " << monitor->scale << "\n"
              << "  Logical Pos: "
                  << monitor->logical_position.x << ","
                  << monitor->logical_position.y << "\n";
    lvkw_display_destroyMonitor(monitor);
}
```

### The Primary Monitor
The "Primary" monitor is the system's default display. On most platforms, this is where new windows appear by default.

## Video Modes

Each monitor supports a set of video modes (resolution and refresh rate).

### Querying Supported Modes
```cpp
auto modes = ctx.getMonitorModes(monitor);
for (const auto& mode : modes) {
    // mode.size.x / mode.size.y (Pixels)
    // mode.refresh_rate_mhz (e.g. 60000 for 60Hz)
}
```

### Changing Resolution (Fullscreen)
To change the resolution of a monitor, you must put a window into fullscreen mode on that monitor. LVKW does not support changing global desktop resolutions outside of a window's context.

```cpp
LVKW_WindowAttributes attrs = {};
attrs.fullscreen = true;
attrs.monitor = monitor;
// Some backends will attempt to match the closest mode to your requested logicalSize
window.update(LVKW_WND_ATTR_FULLSCREEN | LVKW_WND_ATTR_MONITOR, attrs);
```

## Monitor Events

Monitors are dynamic. Users can plug in new displays or change their system-wide resolution at any time.

1.  **`LVKW_EVENT_TYPE_MONITOR_CONNECTION`**: Fired when a monitor is connected or disconnected.
    *   The event carries a borrowed `LVKW_MonitorRef*` (`monitor_ref`).
    *   If you need persistent access, call `lvkw_display_createMonitor(ref, &owned)` and release with `lvkw_display_destroyMonitor(owned)`.
2.  **`LVKW_EVENT_TYPE_MONITOR_MODE`**: Fired when a monitor's current mode, scale, or logical size changes.

## Coordinate Space

LVKW uses a **Global Logical Coordinate Space**.
*   The top-left of the primary monitor is usually `(0, 0)`.
*   Other monitors are positioned relative to it based on the OS's display layout.
*   `monitor->logical_position` tells you where that monitor's top-left corner sits in this global space.
