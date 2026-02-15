# Clipboard Management

## Simple Text Access

For the majority of use cases, you only need to exchange plain text.

### Setting Text
```cpp
window.setClipboardText("Hello from LVKW!");
```

```c
lvkw_wnd_setClipboardText(window, "Hello from LVKW!");
```

**Lifetime Note:** You can safely free or modify your local buffer as soon as the call returns.

### Getting Text
```cpp
const char* text = window.getClipboardText();
if (text) {
    // Process text...
}
```

```c
const char* text;
lvkw_wnd_getClipboardText(window, &text);
```

**Lifetime Note:** The pointer returned by `getClipboardText` is managed by LVKW. It remains valid until the next call to `getClipboardText` **or** `getClipboardData` on any window in the same context, or until the context is destroyed. Do not free it.

## Advanced MIME Data

### Providing Multiple Formats
When you copy something, you can provide multiple representations to maximize compatibility.

```cpp
std::vector<LVKW_ClipboardData> formats;

// Plain text version
formats.push_back({
    .mime_type = "text/plain",
    .data = my_text.data(),
    .size = my_text.size()
});

// HTML version
formats.push_back({
    .mime_type = "text/html",
    .data = my_html.data(),
    .size = my_html.size()
});

window.setClipboardData(formats);
```

### Querying Available Formats
Before requesting data, you can check what the sender has provided.

```cpp
auto types = window.getClipboardMimeTypes();
for (const char* type : types) {
    if (strcmp(type, "image/png") == 0) {
        // We can handle this!
    }
}
```

```c
uint32_t count = 0;
const char** mime_types = NULL;
lvkw_wnd_getClipboardMimeTypes(window, &mime_types, &count);
```

**Lifetime Note:** The MIME array and strings are managed by LVKW. They remain valid until the next call to `getClipboardMimeTypes` on any window in the same context, or until the context is destroyed.

### Retrieving Specific Data
And then retrieve what you want.
```cpp
try {
    auto bytes = window.getClipboardData("image/png");
    // bytes is a std::span<const uint8_t>
    decode_png(bytes.data(), bytes.size());
} catch (const lvkw::Exception& e) {
    // Format not available
}
```

**Behavior Note:** If the requested MIME type is not currently available, `getClipboardData` fails (`LVKW_ERROR` in C, exception in C++).

## Wayland-Specific Notes

On Wayland, `setClipboardText` / `setClipboardData` have additional preconditions:

- A valid recent input serial must be available (from keyboard/pointer interaction).
- `wl_data_device_manager` must be available on the compositor/session.

If these conditions are not met, the set operation fails immediately with diagnostics.

## Thread Safety

Clipboard operations are **primary-thread-only**. You must perform clipboard access on the thread that created the context. This is due to restrictions in underlying display protocols (like Wayland and Win32), where clipboard ownership is tied to the windowing message loop.
