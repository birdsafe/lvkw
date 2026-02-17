# Header Include Policy

LVKW supports two public include styles for both C and C++:

1. Umbrella includes (convenience):
- C: `#include <lvkw/lvkw.h>`
- C++: `#include <lvkw/lvkw.hpp>`

2. First-level domain includes (fully supported and documented):
- C domains:
  - `lvkw/c/core.h`
  - `lvkw/c/context.h`
  - `lvkw/c/display.h`
  - `lvkw/c/events.h`
  - `lvkw/c/input.h`
  - `lvkw/c/data.h`
  - `lvkw/c/instrumentation.h`
  - `lvkw/c/shortcuts.h`
  - `lvkw/c/ext/controller.h`
- C++ domains:
  - `lvkw/cpp/fwd.hpp`
  - `lvkw/cpp/error.hpp`
  - `lvkw/cpp/context.hpp`
  - `lvkw/cpp/window.hpp`
  - `lvkw/cpp/cursor.hpp`
  - `lvkw/cpp/monitor.hpp`
  - `lvkw/cpp/controller.hpp`
  - `lvkw/cpp/events.hpp`
  - `lvkw/cpp/cxx20.hpp`

Only `detail/*` headers are internal implementation details and should not be included directly.
