#ifndef LVKW_WAYLAND_LOADER_H_INCLUDED
#define LVKW_WAYLAND_LOADER_H_INCLUDED

bool lvkw_load_wayland_symbols(void);
void lvkw_unload_wayland_symbols(void);

#ifdef LVKW_ENABLE_DIAGNOSTICS
const char* lvkw_wayland_loader_get_diagnostic(void);
#endif

#endif
