# Ownership Glossary

## Borrowed Ref

A borrowed ref (`LVKW_ControllerRef*`, `LVKW_MonitorRef*`) is a non-owning pointer returned by connection/list APIs. It is used only to bind an owned handle.

## Owned Handle

An owned handle (`LVKW_Controller*`, `LVKW_Monitor*`) is obtained with `create*` APIs and must be released with matching `destroy*` APIs.

## Lost Handle

A lost handle is still valid memory but represents a disconnected resource. It is marked with `*_STATE_LOST`.

## Typical Flow

1. Receive ref from event/list.
2. Convert ref to owned handle with `create*`.
3. Use owned handle.
4. Release owned handle with `destroy*`.
