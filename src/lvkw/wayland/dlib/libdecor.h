#ifndef LVKW_LIBDECOR_DLIB_H_INCLUDED
#define LVKW_LIBDECOR_DLIB_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "dlib/wayland-client.h"
#include "lvkw_internal.h"

/* libdecor types forward declarations */
struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;
struct xdg_toplevel;
struct xdg_surface;
struct wl_display;
struct wl_surface;
struct wl_output;

enum libdecor_error {
  LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
};

enum libdecor_window_state {
  LIBDECOR_WINDOW_STATE_NONE = 0,
  LIBDECOR_WINDOW_STATE_ACTIVE = 1 << 0,
  LIBDECOR_WINDOW_STATE_MAXIMIZED = 1 << 1,
  LIBDECOR_WINDOW_STATE_FULLSCREEN = 1 << 2,
  LIBDECOR_WINDOW_STATE_TILED_LEFT = 1 << 3,
  LIBDECOR_WINDOW_STATE_TILED_RIGHT = 1 << 4,
  LIBDECOR_WINDOW_STATE_TILED_TOP = 1 << 5,
  LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 1 << 6,
  LIBDECOR_WINDOW_STATE_SUSPENDED = 1 << 7,
};

struct libdecor_interface {
  void (*error)(struct libdecor *context, enum libdecor_error error, const char *message);
};

struct libdecor_frame_interface {
  void (*configure)(struct libdecor_frame *frame, struct libdecor_configuration *configuration, void *user_data);
  void (*close)(struct libdecor_frame *frame, void *user_data);
  void (*commit)(struct libdecor_frame *frame, void *user_data);
  void (*dismiss_popup)(struct libdecor_frame *frame, const char *seat_name, void *user_data);
};

#define LVKW_LIBDECOR_FUNCTIONS_TABLE                                                                          \
  LVKW_LIB_FN(struct libdecor *, new, (struct wl_display * display, struct libdecor_interface * interface))     \
  LVKW_LIB_FN(void, unref, (struct libdecor * context))                                                        \
  LVKW_LIB_FN(struct libdecor_frame *, decorate,                                                               \
              (struct libdecor * context, struct wl_surface * surface, struct libdecor_frame_interface * interface, \
               void *user_data))                                                                               \
  LVKW_LIB_FN(void, frame_unref, (struct libdecor_frame * frame))                                              \
  LVKW_LIB_FN(void, frame_set_title, (struct libdecor_frame * frame, const char *title))                        \
  LVKW_LIB_FN(void, frame_set_app_id, (struct libdecor_frame * frame, const char *app_id))                      \
  LVKW_LIB_FN(void, frame_map, (struct libdecor_frame * frame))                                                 \
  LVKW_LIB_FN(void, frame_set_visibility, (struct libdecor_frame * frame, bool visible))                        \
  LVKW_LIB_FN(void, frame_commit,                                                                              \
              (struct libdecor_frame * frame, struct libdecor_state * state,                                   \
               struct libdecor_configuration * configuration))                                                 \
  LVKW_LIB_FN(struct xdg_toplevel *, frame_get_xdg_toplevel, (struct libdecor_frame * frame))                    \
  LVKW_LIB_FN(struct xdg_surface *, frame_get_xdg_surface, (struct libdecor_frame * frame))                      \
  LVKW_LIB_FN(void, frame_set_min_content_size, (struct libdecor_frame * frame, int width, int height))         \
  LVKW_LIB_FN(void, frame_set_max_content_size, (struct libdecor_frame * frame, int width, int height))         \
  LVKW_LIB_FN(void, frame_set_fullscreen, (struct libdecor_frame * frame, struct wl_output * output))            \
  LVKW_LIB_FN(void, frame_unset_fullscreen, (struct libdecor_frame * frame))                                    \
  LVKW_LIB_FN(struct libdecor_state *, state_new, (int width, int height))                                      \
  LVKW_LIB_FN(void, state_free, (struct libdecor_state * state))                                               \
  LVKW_LIB_FN(bool, configuration_get_content_size,                                                            \
              (struct libdecor_configuration * configuration, struct libdecor_frame * frame, int *width,       \
               int *height))                                                                                   \
  LVKW_LIB_FN(bool, configuration_get_window_state,                                                            \
              (struct libdecor_configuration * configuration, enum libdecor_window_state * window_state))      \
  LVKW_LIB_OPT_FN(void, set_userdata, (struct libdecor * context, void *userdata))                             \
  LVKW_LIB_OPT_FN(void *, get_userdata, (struct libdecor * context))                                           \
  // end of table

typedef struct LVKW_Lib_Decor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(ret, name, args) ret (*name) args;
#define LVKW_LIB_OPT_FN(ret, name, args)
  LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
#undef LVKW_LIB_OPT_FN

  struct {
#define LVKW_LIB_FN(ret, name, args)
#define LVKW_LIB_OPT_FN(ret, name, args) ret (*name) args;
    LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
#undef LVKW_LIB_OPT_FN
  } opt;
} LVKW_Lib_Decor;

extern LVKW_Lib_Decor lvkw_lib_decor;

#define libdecor_new(...) lvkw_lib_decor.new(__VA_ARGS__)
#define libdecor_unref(...) lvkw_lib_decor.unref(__VA_ARGS__)
#define libdecor_decorate(...) lvkw_lib_decor.decorate(__VA_ARGS__)
#define libdecor_frame_unref(...) lvkw_lib_decor.frame_unref(__VA_ARGS__)
#define libdecor_frame_set_title(...) lvkw_lib_decor.frame_set_title(__VA_ARGS__)
#define libdecor_frame_set_app_id(...) lvkw_lib_decor.frame_set_app_id(__VA_ARGS__)
#define libdecor_frame_map(...) lvkw_lib_decor.frame_map(__VA_ARGS__)
#define libdecor_frame_set_visibility(...) lvkw_lib_decor.frame_set_visibility(__VA_ARGS__)
#define libdecor_frame_commit(...) lvkw_lib_decor.frame_commit(__VA_ARGS__)
#define libdecor_frame_get_xdg_toplevel(...) lvkw_lib_decor.frame_get_xdg_toplevel(__VA_ARGS__)
#define libdecor_frame_get_xdg_surface(...) lvkw_lib_decor.frame_get_xdg_surface(__VA_ARGS__)
#define libdecor_frame_set_min_content_size(...) lvkw_lib_decor.frame_set_min_content_size(__VA_ARGS__)
#define libdecor_frame_set_max_content_size(...) lvkw_lib_decor.frame_set_max_content_size(__VA_ARGS__)
#define libdecor_frame_set_fullscreen(...) lvkw_lib_decor.frame_set_fullscreen(__VA_ARGS__)
#define libdecor_frame_unset_fullscreen(...) lvkw_lib_decor.frame_unset_fullscreen(__VA_ARGS__)
#define libdecor_state_new(...) lvkw_lib_decor.state_new(__VA_ARGS__)
#define libdecor_state_free(...) lvkw_lib_decor.state_free(__VA_ARGS__)
#define libdecor_configuration_get_content_size(...) lvkw_lib_decor.configuration_get_content_size(__VA_ARGS__)
#define libdecor_configuration_get_window_state(...) lvkw_lib_decor.configuration_get_window_state(__VA_ARGS__)
#define libdecor_set_userdata(...) lvkw_lib_decor.opt.set_userdata(__VA_ARGS__)
#define libdecor_get_userdata(...) lvkw_lib_decor.opt.get_userdata(__VA_ARGS__)

#endif
