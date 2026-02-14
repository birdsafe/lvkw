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

**Lifetime Note:** The pointer returned by `getClipboardText` is managed by LVKW. It remains valid until the next call to `getClipboardText` on the same context or until the context is destroyed. Do not free it.

## Advanced MIME Data

### Providing Multiple Formats
When you copy something, can provide multiple representations to maximize compatibility.

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

## Thread Safety

Clipboard operations are **Main-Thread Bound**. Even if you have the Hybrid threading model enabled, you must perform clipboard access on the thread that created the context. This is due to restrictions in underlying display protocols (like Wayland and Win32) where clipboard ownership is tied to the windowing message loop.
