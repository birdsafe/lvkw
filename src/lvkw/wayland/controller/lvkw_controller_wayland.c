#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "controller/lvkw_controller_internal.h"
#include "lvkw_api_checks.h"
#include "lvkw_diag_internal.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_CONTROLLER_ENABLED

#define LVKW_EV_BUF_SIZE (sizeof(struct inotify_event) + NAME_MAX + 1)

static bool _is_gamepad(int fd) {
  unsigned long ev_bits[EV_MAX / 8 / sizeof(unsigned long) + 1] = {0};
  unsigned long key_bits[KEY_MAX / 8 / sizeof(unsigned long) + 1] = {0};
  unsigned long abs_bits[ABS_MAX / 8 / sizeof(unsigned long) + 1] = {0};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) return false;

  // Gamepads must have keys and absolute axes
  if (!(ev_bits[EV_KEY / 8 / sizeof(unsigned long)] & (1UL << (EV_KEY % (8 * sizeof(unsigned long))))) ||
      !(ev_bits[EV_ABS / 8 / sizeof(unsigned long)] & (1UL << (EV_ABS % (8 * sizeof(unsigned long)))))) {
    return false;
  }

  if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) return false;
  if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) < 0) return false;

  // Check for BTN_GAMEPAD or BTN_SOUTH
  bool has_gamepad_button =
      (key_bits[BTN_GAMEPAD / 8 / sizeof(unsigned long)] & (1UL << (BTN_GAMEPAD % (8 * sizeof(unsigned long))))) ||
      (key_bits[BTN_SOUTH / 8 / sizeof(unsigned long)] & (1UL << (BTN_SOUTH % (8 * sizeof(unsigned long)))));

  // Check for common axes
  bool has_axes = (abs_bits[ABS_X / 8 / sizeof(unsigned long)] & (1UL << (ABS_X % (8 * sizeof(unsigned long))))) &&
                  (abs_bits[ABS_Y / 8 / sizeof(unsigned long)] & (1UL << (ABS_Y % (8 * sizeof(unsigned long)))));

  return has_gamepad_button && has_axes;
}

static void _add_device(LVKW_Context_WL *ctx, const char *path) {
  // Check if already added
  for (struct LVKW_CtrlDevice_WL *d = ctx->controller.devices; d; d = d->next) {
    if (strcmp(d->path, path) == 0) return;
  }

  int fd = -1;
  // Try to open the device with a few retries because it might not be ready
  // immediately after the inotify event.
  for (int i = 0; i < 5; i++) {
    fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd >= 0) break;
    usleep(10000);  // 10ms
  }

  if (fd < 0) return;

  if (!_is_gamepad(fd)) {
    close(fd);
    return;
  }

  struct LVKW_CtrlDevice_WL *dev =
      lvkw_alloc(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, sizeof(struct LVKW_CtrlDevice_WL));
  if (!dev) {
    close(fd);
    return;
  }

  memset(dev, 0, sizeof(*dev));
  dev->fd = fd;
  dev->id = ++ctx->controller.next_id;

  size_t path_len = strlen(path) + 1;
  dev->path = lvkw_alloc(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, path_len);
  if (dev->path) {
    memcpy(dev->path, path, path_len);
  }

  char name[256] = "Unknown Gamepad";
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    strncpy(name, "Unknown Gamepad", sizeof(name));
  }

  size_t name_len = strlen(name) + 1;
  dev->name = lvkw_alloc(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, name_len);
  if (dev->name) {
    memcpy(dev->name, name, name_len);
  }

  struct input_id id_info;
  if (ioctl(fd, EVIOCGID, &id_info) == 0) {
    dev->vendor_id = id_info.vendor;
    dev->product_id = id_info.product;
    dev->version = id_info.version;
  }

  dev->next = ctx->controller.devices;
  ctx->controller.devices = dev;

  // Emit connection event
  LVKW_Event evt = {.type = LVKW_EVENT_TYPE_CONTROLLER_CONNECTION,
                    .controller_connection = {
                        .id = dev->id,
                        .connected = true,
                    }};
  _lvkw_wayland_push_event(ctx, &evt);
}

static void _remove_device(LVKW_Context_WL *ctx, struct LVKW_CtrlDevice_WL *dev, struct LVKW_CtrlDevice_WL *prev) {
  if (prev) {
    prev->next = dev->next;
  }
  else {
    ctx->controller.devices = dev->next;
  }

  // Emit disconnection event
  LVKW_Event evt = {.type = LVKW_EVENT_TYPE_CONTROLLER_CONNECTION,
                    .controller_connection = {
                        .id = dev->id,
                        .connected = false,
                    }};
  _lvkw_wayland_push_event(ctx, &evt);

  close(dev->fd);
  if (dev->name) {
    lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, dev->name);
  }
  if (dev->path) {
    lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, dev->path);
  }
  lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, dev);
}

void _lvkw_ctrl_init_context(LVKW_Context_Base *ctx_base) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_base;

  ctx->controller.inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (ctx->controller.inotify_fd >= 0) {
    inotify_add_watch(ctx->controller.inotify_fd, "/dev/input", IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MOVED_TO);
  }

  // Initial scan
  DIR *dir = opendir("/dev/input");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir))) {
      if (strncmp(ent->d_name, "event", 5) == 0) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
        _add_device(ctx, path);
      }
    }
    closedir(dir);
  }
}

void _lvkw_ctrl_cleanup_context(LVKW_Context_Base *ctx_base) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_base;

  if (ctx->controller.inotify_fd >= 0) {
    close(ctx->controller.inotify_fd);
  }

  struct LVKW_CtrlDevice_WL *curr = ctx->controller.devices;
  while (curr) {
    struct LVKW_CtrlDevice_WL *next = curr->next;
    close(curr->fd);
    if (curr->name) {
      lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, curr->name);
    }
    if (curr->path) {
      lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, curr->path);
    }
    lvkw_free(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, curr);
    curr = next;
  }
}

static bool _process_device_events(LVKW_Context_WL *ctx, struct LVKW_CtrlDevice_WL *dev) {
  struct input_event ev;
  ssize_t rd;
  while ((rd = read(dev->fd, &ev, sizeof(ev))) == sizeof(ev)) {
    if (ev.type == EV_KEY) {
      int btn_idx = -1;
      switch (ev.code) {
        case BTN_SOUTH:
          btn_idx = LVKW_CTRL_BUTTON_SOUTH;
          break;
        case BTN_EAST:
          btn_idx = LVKW_CTRL_BUTTON_EAST;
          break;
        case BTN_WEST:
          btn_idx = LVKW_CTRL_BUTTON_WEST;
          break;
        case BTN_NORTH:
          btn_idx = LVKW_CTRL_BUTTON_NORTH;
          break;
        case BTN_TL:
          btn_idx = LVKW_CTRL_BUTTON_LB;
          break;
        case BTN_TR:
          btn_idx = LVKW_CTRL_BUTTON_RB;
          break;
        case BTN_SELECT:
          btn_idx = LVKW_CTRL_BUTTON_BACK;
          break;
        case BTN_START:
          btn_idx = LVKW_CTRL_BUTTON_START;
          break;
        case BTN_MODE:
          btn_idx = LVKW_CTRL_BUTTON_GUIDE;
          break;
        case BTN_THUMBL:
          btn_idx = LVKW_CTRL_BUTTON_L_THUMB;
          break;
        case BTN_THUMBR:
          btn_idx = LVKW_CTRL_BUTTON_R_THUMB;
          break;
        case BTN_DPAD_UP:
          btn_idx = LVKW_CTRL_BUTTON_DPAD_UP;
          break;
        case BTN_DPAD_DOWN:
          btn_idx = LVKW_CTRL_BUTTON_DPAD_DOWN;
          break;
        case BTN_DPAD_LEFT:
          btn_idx = LVKW_CTRL_BUTTON_DPAD_LEFT;
          break;
        case BTN_DPAD_RIGHT:
          btn_idx = LVKW_CTRL_BUTTON_DPAD_RIGHT;
          break;
      }

      if (btn_idx >= 0 && btn_idx < LVKW_CTRL_BUTTON_STANDARD_COUNT) {
        dev->buttons[btn_idx] = (ev.value != 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
      }
    }
    else if (ev.type == EV_ABS) {
      int axis_idx = -1;
      float val = 0.0f;

      struct input_absinfo abs;
      if (ioctl(dev->fd, (unsigned int)EVIOCGABS(ev.code), &abs) == 0) {
        float range = (float)(abs.maximum - abs.minimum);
        if (range != 0.0f) {
          val = (float)(ev.value - abs.minimum) / range;
          if (ev.code == ABS_X || ev.code == ABS_Y || ev.code == ABS_RX || ev.code == ABS_RY) {
            val = val * 2.0f - 1.0f;  // Map [0, 1] to [-1, 1]
          }
        }
      }

      switch (ev.code) {
        case ABS_X:
          axis_idx = LVKW_CTRL_ANALOG_LEFT_X;
          break;
        case ABS_Y:
          axis_idx = LVKW_CTRL_ANALOG_LEFT_Y;
          break;
        case ABS_RX:
          axis_idx = LVKW_CTRL_ANALOG_RIGHT_X;
          break;
        case ABS_RY:
          axis_idx = LVKW_CTRL_ANALOG_RIGHT_Y;
          break;
        case ABS_Z:
          axis_idx = LVKW_CTRL_ANALOG_LEFT_TRIGGER;
          break;
        case ABS_RZ:
          axis_idx = LVKW_CTRL_ANALOG_RIGHT_TRIGGER;
          break;
        case ABS_HAT0X:
          dev->buttons[LVKW_CTRL_BUTTON_DPAD_LEFT] =
              (ev.value < 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          dev->buttons[LVKW_CTRL_BUTTON_DPAD_RIGHT] =
              (ev.value > 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          break;
        case ABS_HAT0Y:
          dev->buttons[LVKW_CTRL_BUTTON_DPAD_UP] =
              (ev.value < 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          dev->buttons[LVKW_CTRL_BUTTON_DPAD_DOWN] =
              (ev.value > 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          break;
      }

      if (axis_idx >= 0 && axis_idx < LVKW_CTRL_ANALOG_STANDARD_COUNT) {
        dev->analogs[axis_idx].value = val;
      }
    }
  }

  if (rd < 0 && errno != EAGAIN) {
    return false;  // Disconnected
  }
  return true;
}

void _lvkw_ctrl_poll(LVKW_Context_Base *ctx_base) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_base;

  // Poll inotify
  if (ctx->controller.inotify_fd >= 0) {
    char buf[LVKW_EV_BUF_SIZE];
    ssize_t len;
    while ((len = read(ctx->controller.inotify_fd, buf, sizeof(buf))) > 0) {
      for (char *ptr = buf; ptr < buf + len;) {
        struct inotify_event *event = (struct inotify_event *)ptr;
        if (event->len) {
          if (strncmp(event->name, "event", 5) == 0) {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "/dev/input/%s", event->name);
            if (event->mask & (IN_CREATE | IN_MOVED_TO | IN_ATTRIB)) {
              _add_device(ctx, path);
            }
          }
        }
        ptr += sizeof(struct inotify_event) + event->len;
      }
    }
  }

  // Poll devices
  struct LVKW_CtrlDevice_WL *curr = ctx->controller.devices;
  struct LVKW_CtrlDevice_WL *prev = NULL;
  while (curr) {
    if (!_process_device_events(ctx, curr)) {
      struct LVKW_CtrlDevice_WL *to_remove = curr;
      curr = curr->next;
      _remove_device(ctx, to_remove, prev);
      continue;
    }

    prev = curr;
    curr = curr->next;
  }
}

LVKW_Status lvkw_ctrl_create_WL(LVKW_Context *ctx_handle, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  struct LVKW_CtrlDevice_WL *dev = NULL;
  for (struct LVKW_CtrlDevice_WL *d = ctx->controller.devices; d; d = d->next) {
    if (d->id == id) {
      dev = d;
      break;
    }
  }

  if (!dev) return LVKW_ERROR;

  LVKW_Controller_Base *ctrl =
      lvkw_alloc(&ctx->base.prv.alloc_cb, ctx->base.pub.userdata, sizeof(LVKW_Controller_Base));
  if (!ctrl) return LVKW_ERROR;

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->pub.analogs = dev->analogs;
  ctrl->pub.analog_count = LVKW_CTRL_ANALOG_STANDARD_COUNT;
  ctrl->pub.buttons = dev->buttons;
  ctrl->pub.button_count = LVKW_CTRL_BUTTON_STANDARD_COUNT;

  ctrl->prv.ctx_base = &ctx->base;
  ctrl->prv.id = id;

  *out_controller = (LVKW_Controller *)ctrl;
  return LVKW_SUCCESS;
}

void lvkw_ctrl_destroy_WL(LVKW_Controller *controller) {
  if (!controller) return;
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  lvkw_free(&ctrl->prv.ctx_base->prv.alloc_cb, ctrl->prv.ctx_base->pub.userdata, ctrl);
}

LVKW_Status lvkw_ctrl_getInfo_WL(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctrl->prv.ctx_base;

  struct LVKW_CtrlDevice_WL *dev = NULL;
  for (struct LVKW_CtrlDevice_WL *d = ctx->controller.devices; d; d = d->next) {
    if (d->id == ctrl->prv.id) {
      dev = d;
      break;
    }
  }

  if (!dev) return LVKW_ERROR;

  out_info->name = dev->name;
  out_info->vendor_id = dev->vendor_id;
  out_info->product_id = dev->product_id;
  out_info->version = dev->version;
  out_info->is_standardized = true;
  memset(out_info->guid, 0, 16);

  return LVKW_SUCCESS;
}

#endif
