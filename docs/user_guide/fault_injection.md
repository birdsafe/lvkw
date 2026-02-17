# Fault Injection

You can use fault injection to simulate various client configurations without having to track down systems for each permutation.
Their behavior are driven by environment variables. You don't need to alter your app/game in any way. Just enable fault injection 
and control them via the environment variables.

## Enabling Fault Injection

Compile LVKW with the `-DLVKW_ENABLE_FAULT_INJECTION` option.

```bash
cmake -S . -B build -DLVKW_ENABLE_FAULT_INJECTION=ON
cmake --build build
```

When `LVKW_ENABLE_FAULT_INJECTION` is `OFF`, fault-injection environment variables are ignored.

## Wayland: Pretend Protocols Are Missing

you can force protocol globals to be treated as unavailable during registry binding.

Environment variable:

- `LVKW_WAYLAND_PRETEND_MISSING`

Accepted values:

- Comma-separated protocol interface names.

Examples:

```bash
LVKW_WAYLAND_PRETEND_MISSING=wl_data_device_manager ./your_app
LVKW_WAYLAND_PRETEND_MISSING=wp_viewporter,wp_fractional_scale_manager_v1 ./your_app
```

### Notes

- Marking required protocols as missing will typically make context creation fail.
