Enabling `LVKW_ENABLE_INTERNAL_CHECKS` may introduce behavior that is intentionally outside normal runtime guarantees.

## X11: `XSetErrorHandler` is installed

When `LVKW_ENABLE_INTERNAL_CHECKS` is enabled (with diagnostics), LVKW installs an X11 error handler via `XSetErrorHandler`.

This is process-global state in Xlib. It is useful for deep debugging, but it does break the hard "No Dynamic Global state" the library promises. 

If your debugging setup requires multiple concurrent X11 contexts, consider disabling this hook locally while investigating.
