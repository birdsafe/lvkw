# User Guide

This guide is a collection of deep-dives on technical topics and advanced features. For the primary API reference and core usage contracts, please refer to the [public header files](#additional-resources).

Implementation status differs by backend for some advanced domains (clipboard, cursors, monitors, controllers). Each page calls out current backend support explicitly.

## Table of contents

- [Integration Guide](integration.md) - Learn how to add LVKW to your project using CMake, prebuilt binaries, and how to handle system dependencies and Vulkan loading.
- [The Event System & Input](events_and_input.md) - Deep dive into the event queue, tail compression, and input layers (Physical Keys vs Text Input vs IME).
- [Coordinate Systems & High DPI](coordinates_and_dpi.md) - Understanding the difference between Logical Units (UI/Window) and Pixel Units (Rendering), and handling DPI scaling.
- [Monitor & Display Management](monitors.md) - Enumerating monitors, querying video modes, and understanding global logical coordinates.
- [Hardware Cursors](cursors.md) - Using standard system cursors, creating custom cursors from pixels, and cursor visibility modes.
- [Clipboard Management](clipboard.md) - Exchanging text and MIME-based data with the system clipboard.
- [Threading Deep Dive](threading.md) - Full concurrency contract: primary-thread APIs, any-thread APIs, and required synchronization patterns.
- [Metrics & Monitoring](metrics.md) - How to use the metrics API to monitor event queue watermarks and dropped events.
- [Advanced Configuration & Tuning](tuning.md) - Custom Vulkan loading, Wayland decoration modes, and event queue performance tuning.
- [Controller Support](controller.md) - Using gamepads, hotplugging, button mapping, and haptics.
- [Ownership Glossary](ownership_glossary.md) - Borrowed refs vs owned handles for monitors and controllers.
- [Header Include Policy](headers.md) - Supported umbrella and first-level include paths for C and C++.
- [Backend Limitations](backend_limitations.md) - Current backend-specific gaps and protocol-dependent behavior by module.

## Additional Resources

- **API Reference**: The primary documentation for the API is located in the header files:
    - C API: [`include/lvkw/lvkw.h`](../../include/lvkw/lvkw.h)
    - C++ API: [`include/lvkw/lvkw.hpp`](../../include/lvkw/lvkw.hpp)
    - First-level C/C++ domains: [`include/lvkw/c/`](../../include/lvkw/c/) and [`include/lvkw/cpp/`](../../include/lvkw/cpp/)
- **Examples**: Complete usage examples can be found in the [`examples/`](../../examples/) directory.
