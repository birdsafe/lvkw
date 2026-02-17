# Controller Support

## Getting Started

Controller support is optional and guarded by `LVKW_ENABLE_CONTROLLER`. It is included by default, but you can turn it off if you don't need them. This needs to be set when building lvkw itself.

### Hotplugging

Controllers are not available immediately at startup. You must listen for connection events.

1.  **Poll for `LVKW_EVENT_TYPE_CONTROLLER_CONNECTION`:**
    This event fires when a controller is plugged in or removed.
2.  **Create an owned controller handle if you keep it:**
    The event provides a borrowed `event.controller_connection.controller_ref`. If you store it beyond callback scope, call `lvkw_input_createController(ref, &out_controller)`.
3.  **Release when done:**
    Call `lvkw_input_destroyController()` when you no longer need that stored handle.

**Example:**

```cpp
lvkw::pumpEvents(ctx, 0);
lvkw::commitEvents(ctx);
lvkw::scanEvents(ctx, [&](lvkw::ControllerConnectionEvent evt) {
    if (evt->connected) {
        LVKW_Controller* controller = nullptr;
        lvkw_input_createController(evt->controller_ref, &controller);
        // store controller pointer...
    } else {
        // remove stored controller matching evt->controller_ref
        // and release the associated handle on the primary thread
        lvkw_input_destroyController(stored_controller);
    }
});
```

### The Lifetime Model: Why Explicit Creation?

You might wonder why `LVKW_EVENT_TYPE_CONTROLLER_CONNECTION` doesn't just provide a ready-to-use controller pointer.

The reason is explicit ownership and backend-agnostic lifetime control.

Controller connection/disconnection is asynchronous, while handle usage is explicit API traffic. Keeping creation/destruction explicit avoids hidden ownership transitions in event payloads.

LVKW uses borrowed refs and explicit owned handles:

1.  **Connection Event:** `connected=true` gives you a borrowed `LVKW_ControllerRef*`.
2.  **Create Ownership:** call `lvkw_input_createController(ref, &owned)` to acquire an owned `LVKW_Controller*`.
3.  **Disconnection Event:** `connected=false` marks owned handles as lost.
4.  **Explicit Ownership:** destroy each owned handle with `lvkw_input_destroyController()`.

Disconnected handles remain readable for metadata/state but device-backed operations may return errors.

### Button Mapping (`LVKW_CtrlButton`)

LVKW standardizes controller inputs to a common layout (similar to Xbox/DualShock).

*   **`LVKW_CTRL_BUTTON_SOUTH` (A/Cross)**
*   **`LVKW_CTRL_BUTTON_EAST` (B/Circle)**
*   **`LVKW_CTRL_BUTTON_WEST` (X/Square)**
*   **`LVKW_CTRL_BUTTON_NORTH` (Y/Triangle)**
*   **`LVKW_CTRL_BUTTON_LB` / `LVKW_CTRL_BUTTON_RB` (Bumpers)**
*   **`LVKW_CTRL_BUTTON_BACK` / `LVKW_CTRL_BUTTON_START` (Select/Start)**
*   **`LVKW_CTRL_BUTTON_GUIDE` (Home/Xbox)**
*   **`LVKW_CTRL_BUTTON_L_THUMB` / `R_THUMB` (Stick Clicks)**
*   **`LVKW_CTRL_BUTTON_DPAD_UP` / `DPAD_DOWN` / `DPAD_LEFT` / `DPAD_RIGHT`**

**Note:** Always use these constants. Do not rely on raw indices or platform-specific mappings.

### Analog Sticks & Triggers (`LVKW_CtrlAnalog`)

Analog inputs are normalized to the range `[-1.0, 1.0]` for sticks and `[0.0, 1.0]` for triggers.

*   **`LVKW_CTRL_ANALOG_LEFT_X` / `Y`**
*   **`LVKW_CTRL_ANALOG_RIGHT_X` / `Y`**
*   **`LVKW_CTRL_ANALOG_LEFT_TRIGGER`**
*   **`LVKW_CTRL_ANALOG_RIGHT_TRIGGER`**

## Haptics (Rumble)

LVKW supports dual-motor rumble and independent trigger haptics (on supported hardware like DualSense).

**Channels (`LVKW_CtrlHaptic`):**

*   **`LVKW_CTRL_HAPTIC_LOW_FREQ`:** Large motor (Heavy rumble).
*   **`LVKW_CTRL_HAPTIC_HIGH_FREQ`:** Small motor (Subtle/High-pitch rumble).
*   **`LVKW_CTRL_HAPTIC_LEFT_TRIGGER`:** Independent trigger feedback.
*   **`LVKW_CTRL_HAPTIC_RIGHT_TRIGGER`:** Independent trigger feedback.

**Usage:**

```cpp
// Set standard rumble (50% low freq, 80% high freq)
controller.setRumble(0.5f, 0.8f);

// Or specifically target triggers
float trigger_intensities[] = {0.2f, 0.2f};
controller.setHapticLevels(LVKW_CTRL_HAPTIC_LEFT_TRIGGER, trigger_intensities);
```

**Threading Note:**
`lvkw_input_createController`, `lvkw_input_destroyController`, `lvkw_input_getControllerInfo`, and `lvkw_input_setControllerHapticLevels` are primary-thread-only.

## Standard vs. Extended Ranges
LVKW guarantees that the first few indices of buttons, analogs, and haptics correspond to the standard layouts defined above (e.g., `LVKW_CTRL_BUTTON_SOUTH` is always index 0). This allows you to support most controllers out of the box without any configuration.

However, complex devices (like flight sticks, sim rigs, or MMO mice) may have many more inputs than the standard gamepad layout.

The standard indices always map to *something*, but that something might effectively be a "dummy" control if the physical feature is missing.

*   **Reading:** Querying a missing standard button (e.g., D-Pad on a joystick) safely returns `false` or `0.0`.
*   **Writing:** Setting haptics on a missing motor results in a no-op.

No such guarantees exist for indices at or above `LVKW_CTRL_[BUTTON|ANALOG|HAPTIC]_STANDARD_COUNT`.


### Extended Inputs
Indices equal to or greater than `LVKW_CTRL_BUTTON_STANDARD_COUNT` (or `ANALOG`, `HAPTIC`) are considered **Extended Inputs**. These map to extra buttons or axes provided by the hardware driver in no particular order.

### Using Metadata

To make use of these extended inputs, you can inspect the controller's metadata at runtime. The `LVKW_Controller` struct provides arrays of channel info that contain human-readable names.

```cpp
// Check for extra buttons
for (uint32_t i = LVKW_CTRL_BUTTON_STANDARD_COUNT; i < controller->button_count; ++i) {
    printf("Found extra button %u: %s\n", i, controller->button_channels[i].name);
    // Output: "Found extra button 15: Paddle Left"
}

// Check for extra axes (e.g. Throttle)
for (uint32_t i = LVKW_CTRL_ANALOG_STANDARD_COUNT; i < controller->analog_count; ++i) {
    if (strstr(controller->analog_channels[i].name, "Throttle")) {
        // Found a throttle axis!
    }
}
```
