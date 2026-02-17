# Backend Limitations

Quick reference for backend-specific API gotchas.

- `(AS DESIGNED)` means this is expected behavior, not a planned fix.
- Win32 is omitted here because it's still mostly stubbed.

## Table of Contents

- [Wayland](#backend-wayland)
  - [Context](#wayland-module-context)
  - [Display](#wayland-module-display)
  - [Events](#wayland-module-events)
  - [Data](#wayland-module-data)
  - [EXT: Controllers](#wayland-module-ext-controllers)
- [X11](#backend-x11)
  - [Context](#x11-module-context)
  - [Display](#x11-module-display)
  - [Events](#x11-module-events)
  - [Data](#x11-module-data)
  - [EXT: Controllers](#x11-module-ext-controllers)
- [Cocoa](#backend-cocoa)
  - [Context](#cocoa-module-context)
  - [Display](#cocoa-module-display)
  - [Events](#cocoa-module-events)
  - [Data](#cocoa-module-data)
  - [EXT: Controllers](#cocoa-module-ext-controllers)

<a id="backend-wayland"></a>
## Wayland

<a id="wayland-module-context"></a>
### Context

- `(AS DESIGNED)` `LVKW_CONTEXT_ATTR_INHIBIT_IDLE` only works when `zwp_idle_inhibit_manager_v1` is available.
- `(AS DESIGNED)` `LVKW_EVENT_TYPE_IDLE_STATE_CHANGED` / `LVKW_IdleEvent` only show up when `ext-idle-notify-v1` is available.

<a id="wayland-module-display"></a>
### Display

- `LVKW_WINDOW_ATTR_ASPECT_RATIO` is currently stored but not enforced by Wayland size constraints.
- `LVKW_WINDOW_ATTR_PRIMARY_SELECTION` does nothing.

<a id="wayland-module-events"></a>
### Events

- `LVKW_MouseScrollEvent.steps` is not populated. 

<a id="wayland-module-data"></a>
### Data

- Pulling selection/clipboard data (`lvkw_data_pullText`, `lvkw_data_pullData`, `lvkw_data_listBufferMimeTypes`) can stall event responsiveness on slow/large transfers.
- `LVKW_DATA_EXCHANGE_TARGET_PRIMARY` is not implemented; only `LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD` is supported.

<a id="wayland-module-ext-controllers"></a>
### EXT: Controllers

- Controller channels are limited to the standardized layouts (`LVKW_CTRL_HAPTIC_STANDARD_COUNT` / `LVKW_CTRL_ANALOG_STANDARD_COUNT` / `LVKW_CTRL_BUTTON_STANDARD_COUNT`). Extra device-specific channels are still missing.

<a id="backend-x11"></a>
## X11

<a id="x11-module-context"></a>
### Context

- `(AS DESIGNED)` idle inhibition and idle events (`LVKW_CONTEXT_ATTR_INHIBIT_IDLE`, `LVKW_EVENT_TYPE_IDLE_STATE_CHANGED`) depend on XScreenSaver extension availability.

<a id="x11-module-display"></a>
### Display

- `LVKW_WINDOW_ATTR_MONITOR` (monitor targeting) is not implemented.
- `LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH` is not implemented.
- `LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE` hints are not implemented.
- `LVKW_WINDOW_ATTR_TEXT_INPUT_RECT` hint is not implemented.
- `LVKW_WINDOW_ATTR_PRIMARY_SELECTION` is accepted but currently does nothing.

<a id="x11-module-events"></a>
### Events

- `LVKW_EVENT_TYPE_DND_HOVER`, `LVKW_EVENT_TYPE_DND_LEAVE`, and `LVKW_EVENT_TYPE_DND_DROP` are not emitted on X11.
- `LVKW_EVENT_TYPE_TEXT_COMPOSITION` is not emitted on X11.

<a id="x11-module-data"></a>
### Data

- `LVKW_DATA_EXCHANGE_TARGET_PRIMARY` is not implemented; calls return error for that target.

<a id="x11-module-ext-controllers"></a>
### EXT: Controllers

- Controller channels are limited to the standardized layouts (`LVKW_CTRL_HAPTIC_STANDARD_COUNT` / `LVKW_CTRL_ANALOG_STANDARD_COUNT` / `LVKW_CTRL_BUTTON_STANDARD_COUNT`). Extra device-specific channels are still missing.

<a id="backend-cocoa"></a>
## Cocoa

<a id="cocoa-module-context"></a>
### Context

- `lvkw_context_update` currently does not apply `LVKW_CONTEXT_ATTR_INHIBIT_IDLE`, `LVKW_CONTEXT_ATTR_DIAGNOSTICS`, or `LVKW_CONTEXT_ATTR_EVENT_MASK` on Cocoa.

<a id="cocoa-module-display"></a>
### Display

- `lvkw_display_listMonitors` / `lvkw_display_listMonitorModes` are not implemented (monitor queries are effectively empty).
- `lvkw_display_createCursor` / `lvkw_display_getStandardCursor` are not implemented.
- `lvkw_display_updateWindow` currently ignores: `LVKW_WINDOW_ATTR_CURSOR_MODE`, `LVKW_WINDOW_ATTR_CURSOR`, `LVKW_WINDOW_ATTR_MONITOR`, `LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH`, `LVKW_WINDOW_ATTR_ACCEPT_DND`, `LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE`, `LVKW_WINDOW_ATTR_TEXT_INPUT_RECT`, and `LVKW_WINDOW_ATTR_PRIMARY_SELECTION`.

<a id="cocoa-module-events"></a>
### Events

- Cocoa does not currently emit `LVKW_EVENT_TYPE_IDLE_STATE_CHANGED`, `LVKW_EVENT_TYPE_MONITOR_CONNECTION`, `LVKW_EVENT_TYPE_MONITOR_MODE`, `LVKW_EVENT_TYPE_TEXT_INPUT`, `LVKW_EVENT_TYPE_TEXT_COMPOSITION`, `LVKW_EVENT_TYPE_DND_HOVER`, `LVKW_EVENT_TYPE_DND_LEAVE`, `LVKW_EVENT_TYPE_DND_DROP`, or `LVKW_EVENT_TYPE_WINDOW_MAXIMIZED`.

<a id="cocoa-module-data"></a>
### Data

- Clipboard/data APIs (`lvkw_data_set/getClipboardText`, `lvkw_data_set/getClipboardData`, `lvkw_data_getClipboardMimeTypes`, and target-based push/pull/list variants) are not implemented.
- Primary-selection behavior (`LVKW_DATA_EXCHANGE_TARGET_PRIMARY`, `LVKW_WINDOW_ATTR_PRIMARY_SELECTION`) is not implemented.

<a id="cocoa-module-ext-controllers"></a>
### EXT: Controllers

- `lvkw_input_createController`, `lvkw_input_listControllers`, `lvkw_input_getControllerInfo`, and `lvkw_input_setControllerHapticLevels` currently return error on Cocoa.

