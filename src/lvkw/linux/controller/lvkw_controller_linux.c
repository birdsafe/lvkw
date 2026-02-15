// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

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
#include "lvkw/lvkw-core.h"
#include "lvkw_api_constraints.h"
#include "lvkw_diagnostic_internal.h"
#include "lvkw_linux_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER

#define LVKW_EV_BUF_SIZE (sizeof(struct inotify_event) + NAME_MAX + 1)

static LVKW_ControllerContext_Linux *_get_ctrl_ctx(LVKW_Context *ctx_handle) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  // On Linux, both Context_WL and Context_X11 have 'controller' as the first member after 'base'
  return (LVKW_ControllerContext_Linux *)(ctx_base + 1);
}

static bool _is_gamepad(int fd) {
  unsigned long ev_bits[EV_MAX / 8 / sizeof(unsigned long) + 1] = {0};
  unsigned long key_bits[KEY_MAX / 8 / sizeof(unsigned long) + 1] = {0};
  unsigned long abs_bits[ABS_MAX / 8 / sizeof(unsigned long) + 1] = {0};

  if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) return false;

  if (!(ev_bits[EV_KEY / 8 / sizeof(unsigned long)] &
        (1UL << (EV_KEY % (8 * sizeof(unsigned long))))) ||
      !(ev_bits[EV_ABS / 8 / sizeof(unsigned long)] &
        (1UL << (EV_ABS % (8 * sizeof(unsigned long)))))) {
    return false;
  }

  if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) return false;
  if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) < 0) return false;

  bool has_gamepad_button = (key_bits[BTN_GAMEPAD / 8 / sizeof(unsigned long)] &
                             (1UL << (BTN_GAMEPAD % (8 * sizeof(unsigned long))))) ||
                            (key_bits[BTN_SOUTH / 8 / sizeof(unsigned long)] &
                             (1UL << (BTN_SOUTH % (8 * sizeof(unsigned long)))));

  bool has_axes = (abs_bits[ABS_X / 8 / sizeof(unsigned long)] &
                   (1UL << (ABS_X % (8 * sizeof(unsigned long))))) &&
                  (abs_bits[ABS_Y / 8 / sizeof(unsigned long)] &
                   (1UL << (ABS_Y % (8 * sizeof(unsigned long)))));

  return has_gamepad_button && has_axes;
}

static void _add_device(LVKW_Context_Base *ctx_base, LVKW_ControllerContext_Linux *ctrl_ctx,
                        const char *path) {
  for (struct LVKW_CtrlDevice_Linux *d = ctrl_ctx->devices; d; d = d->next) {
    if (strcmp(d->path, path) == 0) return;
  }

  int fd = -1;
  for (int i = 0; i < 5; i++) {
    fd = open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    }
    if (fd >= 0) break;
    usleep(10000);
  }

  if (fd < 0) return;

  if (!_is_gamepad(fd)) {
    close(fd);
    return;
  }

  struct LVKW_CtrlDevice_Linux *dev = lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata,
                                                 sizeof(struct LVKW_CtrlDevice_Linux));
  if (!dev) {
    close(fd);
    return;
  }

  memset(dev, 0, sizeof(*dev));
  dev->fd = fd;
  dev->rumble_effect_id = -1;
  dev->id = ++ctrl_ctx->next_id;

  size_t path_len = strlen(path) + 1;
  dev->path = lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, path_len);
  if (dev->path) {
    memcpy(dev->path, path, path_len);
  }

  char name[256] = "Unknown Gamepad";
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    strncpy(name, "Unknown Gamepad", sizeof(name));
  }

  size_t name_len = strlen(name) + 1;
  dev->name = lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, name_len);
  if (dev->name) {
    memcpy(dev->name, name, name_len);
  }

  struct input_id id_info;
  if (ioctl(fd, EVIOCGID, &id_info) == 0) {
    dev->vendor_id = id_info.vendor;
    dev->product_id = id_info.product;
    dev->version = id_info.version;
  }

  dev->next = ctrl_ctx->devices;
  ctrl_ctx->devices = dev;

  if (ctrl_ctx->push_event) {
    LVKW_Event evt = {.controller_connection = {
                          .id = dev->id,
                          .connected = true,
                      }};
    ctrl_ctx->push_event(ctx_base, LVKW_EVENT_TYPE_CONTROLLER_CONNECTION, NULL, &evt);
  }
}

static void _remove_device(LVKW_Context_Base *ctx_base, LVKW_ControllerContext_Linux *ctrl_ctx,
                           struct LVKW_CtrlDevice_Linux *dev, struct LVKW_CtrlDevice_Linux *prev) {
  if (prev) {
    prev->next = dev->next;
  }
  else {
    ctrl_ctx->devices = dev->next;
  }

  if (ctrl_ctx->push_event) {
    LVKW_Event evt = {.controller_connection = {
                          .id = dev->id,
                          .connected = false,
                      }};
    ctrl_ctx->push_event(ctx_base, LVKW_EVENT_TYPE_CONTROLLER_CONNECTION, NULL, &evt);
  }

  if (dev->rumble_effect_id >= 0) {
    ioctl(dev->fd, EVIOCRMFF, dev->rumble_effect_id);
  }

  close(dev->fd);
  if (dev->name) {
    lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, dev->name);
  }
  if (dev->path) {
    lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, dev->path);
  }
  lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, dev);
}

void _lvkw_ctrl_init_context_Linux(LVKW_Context_Base *ctx_base,
                                   LVKW_ControllerContext_Linux *ctrl_ctx,
                                   void (*push_event)(LVKW_Context_Base *ctx, LVKW_EventType type,
                                                      LVKW_Window *window, const LVKW_Event *evt)) {
  memset(ctrl_ctx, 0, sizeof(*ctrl_ctx));
  ctrl_ctx->push_event = push_event;

  ctrl_ctx->inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (ctrl_ctx->inotify_fd >= 0) {
    inotify_add_watch(ctrl_ctx->inotify_fd, "/dev/input",
                      IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MOVED_TO);
  }

  DIR *dir = opendir("/dev/input");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir))) {
      if (strncmp(ent->d_name, "event", 5) == 0) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
        _add_device(ctx_base, ctrl_ctx, path);
      }
    }
    closedir(dir);
  }
}

void _lvkw_ctrl_cleanup_context_Linux(LVKW_Context_Base *ctx_base,
                                      LVKW_ControllerContext_Linux *ctrl_ctx) {
  if (ctrl_ctx->inotify_fd >= 0) {
    close(ctrl_ctx->inotify_fd);
  }

  struct LVKW_CtrlDevice_Linux *curr = ctrl_ctx->devices;
  while (curr) {
    struct LVKW_CtrlDevice_Linux *next = curr->next;
    if (curr->rumble_effect_id >= 0) {
      ioctl(curr->fd, EVIOCRMFF, curr->rumble_effect_id);
    }
    close(curr->fd);
    if (curr->name) {
      lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, curr->name);
    }
    if (curr->path) {
      lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, curr->path);
    }
    lvkw_free(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, curr);
    curr = next;
  }
}

static bool _process_device_events(struct LVKW_CtrlDevice_Linux *dev) {
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
        dev->buttons[btn_idx] =
            (ev.value != 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
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
            val = val * 2.0f - 1.0f;
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
  if (rd < 0 && errno != EAGAIN) return false;
  return true;
}

void _lvkw_ctrl_poll_Linux(LVKW_Context_Base *ctx_base, LVKW_ControllerContext_Linux *ctrl_ctx) {
  if (ctrl_ctx->inotify_fd >= 0) {
    char buf[LVKW_EV_BUF_SIZE];
    ssize_t len;
    while ((len = read(ctrl_ctx->inotify_fd, buf, sizeof(buf))) > 0) {
      for (char *ptr = buf; ptr < buf + len;) {
        struct inotify_event *event = (struct inotify_event *)ptr;
        if (event->len && (strncmp(event->name, "event", 5) == 0)) {
          char path[PATH_MAX];
          snprintf(path, sizeof(path), "/dev/input/%s", event->name);
          if (event->mask & (IN_CREATE | IN_MOVED_TO | IN_ATTRIB)) {
            _add_device(ctx_base, ctrl_ctx, path);
          }
        }
        ptr += sizeof(struct inotify_event) + event->len;
      }
    }
  }

  struct LVKW_CtrlDevice_Linux *curr = ctrl_ctx->devices;
  struct LVKW_CtrlDevice_Linux *prev = NULL;
  while (curr) {
    if (!_process_device_events(curr)) {
      struct LVKW_CtrlDevice_Linux *to_remove = curr;
      curr = curr->next;
      _remove_device(ctx_base, ctrl_ctx, to_remove, prev);
      continue;
    }
    prev = curr;
    curr = curr->next;
  }
}

LVKW_Status lvkw_ctrl_create_Linux(LVKW_Context *ctx_handle, LVKW_CtrlId id,
                                   LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, ctx_handle, id, out_controller);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  LVKW_ControllerContext_Linux *ctrl_ctx = _get_ctrl_ctx(ctx_handle);

  struct LVKW_CtrlDevice_Linux *dev = NULL;
  for (struct LVKW_CtrlDevice_Linux *d = ctrl_ctx->devices; d; d = d->next) {
    if (d->id == id) {
      dev = d;
      break;
    }
  }

  if (!dev) return LVKW_ERROR;

  LVKW_Controller_Base *ctrl =
      lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, sizeof(LVKW_Controller_Base));
  if (!ctrl) return LVKW_ERROR;

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->pub.analogs = dev->analogs;
  ctrl->pub.analog_count = LVKW_CTRL_ANALOG_STANDARD_COUNT;
  ctrl->pub.buttons = dev->buttons;
  ctrl->pub.button_count = LVKW_CTRL_BUTTON_STANDARD_COUNT;
  ctrl->pub.haptic_count = 0;

  unsigned long ff_bits[FF_MAX / (8 * sizeof(unsigned long)) + 1] = {0};
  if (ioctl(dev->fd, EVIOCGBIT(EV_FF, sizeof(ff_bits)), ff_bits) >= 0) {
    bool has_rumble = ff_bits[FF_RUMBLE / (8 * sizeof(unsigned long))] &
                      (1UL << (FF_RUMBLE % (8 * sizeof(unsigned long))));
    if (has_rumble) {
      ctrl->pub.haptic_count = 2;
      ctrl->prv.haptic_channels_backing = lvkw_alloc(
          &ctx_base->prv.alloc_cb, ctx_base->pub.userdata,
          sizeof(LVKW_HapticChannelInfo) * ctrl->pub.haptic_count);
      if (ctrl->prv.haptic_channels_backing) {
        ctrl->prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_LOW_FREQ].name =
            "Low Frequency";
        ctrl->prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_HIGH_FREQ].name =
            "High Frequency";
        ctrl->pub.haptic_channels = ctrl->prv.haptic_channels_backing;
      }
    }
  }

  ctrl->prv.ctx_base = ctx_base;
  ctrl->prv.id = id;

  *out_controller = (LVKW_Controller *)ctrl;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_destroy_Linux(LVKW_Controller *controller) {
  LVKW_API_VALIDATE(ctrl_destroy, controller);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  if (ctrl->prv.analog_channels_backing) {
    lvkw_free(&ctrl->prv.ctx_base->prv.alloc_cb, ctrl->prv.ctx_base->pub.userdata,
              ctrl->prv.analog_channels_backing);
  }
  if (ctrl->prv.button_channels_backing) {
    lvkw_free(&ctrl->prv.ctx_base->prv.alloc_cb, ctrl->prv.ctx_base->pub.userdata,
              ctrl->prv.button_channels_backing);
  }
  if (ctrl->prv.haptic_channels_backing) {
    lvkw_free(&ctrl->prv.ctx_base->prv.alloc_cb, ctrl->prv.ctx_base->pub.userdata,
              ctrl->prv.haptic_channels_backing);
  }
  lvkw_free(&ctrl->prv.ctx_base->prv.alloc_cb, ctrl->prv.ctx_base->pub.userdata, ctrl);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_getInfo_Linux(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  LVKW_ControllerContext_Linux *ctrl_ctx = _get_ctrl_ctx((LVKW_Context *)ctrl->prv.ctx_base);

  struct LVKW_CtrlDevice_Linux *dev = NULL;
  for (struct LVKW_CtrlDevice_Linux *d = ctrl_ctx->devices; d; d = d->next) {
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

LVKW_Status lvkw_ctrl_setHapticLevels_Linux(LVKW_Controller *controller, uint32_t first_haptic,
                                            uint32_t count, const LVKW_real_t *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);

  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  LVKW_ControllerContext_Linux *ctrl_ctx = _get_ctrl_ctx((LVKW_Context *)ctrl->prv.ctx_base);

  struct LVKW_CtrlDevice_Linux *dev = NULL;
  for (struct LVKW_CtrlDevice_Linux *d = ctrl_ctx->devices; d; d = d->next) {
    if (d->id == ctrl->prv.id) {
      dev = d;
      break;
    }
  }

  if (!dev) return LVKW_ERROR;

  LVKW_real_t low = 0.0f;
  LVKW_real_t high = 0.0f;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t channel = first_haptic + i;
    if (channel == LVKW_CTRL_HAPTIC_LOW_FREQ) {
      low = intensities[i];
    }
    else if (channel == LVKW_CTRL_HAPTIC_HIGH_FREQ) {
      high = intensities[i];
    }
  }

  struct ff_effect effect = {
      .type = FF_RUMBLE,
      .id = (short)dev->rumble_effect_id,
      .u.rumble =
          {
              .strong_magnitude = (uint16_t)(low * 65535.0f),
              .weak_magnitude = (uint16_t)(high * 65535.0f),
          },
      .replay =
          {
              .length = 200,
              .delay = 0,
          },
  };

  if (ioctl(dev->fd, EVIOCSFF, &effect) < 0) return LVKW_ERROR;
  dev->rumble_effect_id = effect.id;

  struct input_event play = {
      .type = EV_FF,
      .code = (uint16_t)effect.id,
      .value = 1,
  };
  if (write(dev->fd, &play, sizeof(play)) != (ssize_t)sizeof(play)) return LVKW_ERROR;

  return LVKW_SUCCESS;
}

#endif
