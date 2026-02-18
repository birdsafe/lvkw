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

#include "controller/controller_internal.h"
#include "lvkw/c/core.h"
#include "api_constraints.h"
#include "diagnostic_internal.h"
#include "linux_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER

#define LVKW_EV_BUF_SIZE (sizeof(struct inotify_event) + NAME_MAX + 1)

static LVKW_ControllerContext_Linux *_get_ctrl_ctx(LVKW_Context *ctx_handle) {
  LVKW_Context_Linux *linux_ctx = (LVKW_Context_Linux *)ctx_handle;
#ifdef LVKW_ENABLE_CONTROLLER
  return &linux_ctx->controller;
#else
  (void)linux_ctx;
  return NULL;
#endif
}

static void _remove_device(LVKW_Context_Base *ctx_base, LVKW_ControllerContext_Linux *ctrl_ctx,
                           struct LVKW_CtrlDevice_Linux *dev, struct LVKW_CtrlDevice_Linux *prev);

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

static LVKW_Controller_Base *_alloc_controller(LVKW_Context_Base *ctx_base,
                                               struct LVKW_CtrlDevice_Linux *dev) {
  LVKW_Controller_Base *ctrl = lvkw_context_alloc(ctx_base, sizeof(LVKW_Controller_Base));
  if (!ctrl) return NULL;

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->pub.context = &ctx_base->pub;
  ctrl->pub.analog_count = LVKW_CTRL_ANALOG_STANDARD_COUNT;
  ctrl->pub.button_count = LVKW_CTRL_BUTTON_STANDARD_COUNT;
  ctrl->pub.haptic_count = 0;

  ctrl->prv.analogs_backing =
      lvkw_context_alloc(ctx_base, sizeof(LVKW_AnalogInputState) * ctrl->pub.analog_count);
  ctrl->prv.buttons_backing =
      lvkw_context_alloc(ctx_base, sizeof(LVKW_ButtonState) * ctrl->pub.button_count);
  if (!ctrl->prv.analogs_backing || !ctrl->prv.buttons_backing) {
    if (ctrl->prv.analogs_backing) lvkw_context_free(ctx_base, ctrl->prv.analogs_backing);
    if (ctrl->prv.buttons_backing) lvkw_context_free(ctx_base, ctrl->prv.buttons_backing);
    lvkw_context_free(ctx_base, ctrl);
    return NULL;
  }
  memset(ctrl->prv.analogs_backing, 0, sizeof(LVKW_AnalogInputState) * ctrl->pub.analog_count);
  memset(ctrl->prv.buttons_backing, 0, sizeof(LVKW_ButtonState) * ctrl->pub.button_count);
  ctrl->pub.analogs = ctrl->prv.analogs_backing;
  ctrl->pub.buttons = ctrl->prv.buttons_backing;

  ctrl->prv.analog_channels_backing = lvkw_context_alloc(
      ctx_base, sizeof(LVKW_AnalogChannelInfo) * ctrl->pub.analog_count);
  if (ctrl->prv.analog_channels_backing) {
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_X].name = "Left X";
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_Y].name = "Left Y";
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_X].name = "Right X";
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_Y].name = "Right Y";
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_TRIGGER].name = "Left Trigger";
    ctrl->prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_TRIGGER].name = "Right Trigger";
    ctrl->pub.analog_channels = ctrl->prv.analog_channels_backing;
  }

  ctrl->prv.button_channels_backing = lvkw_context_alloc(
      ctx_base, sizeof(LVKW_ButtonChannelInfo) * ctrl->pub.button_count);
  if (ctrl->prv.button_channels_backing) {
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_SOUTH].name = "South";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_EAST].name = "East";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_WEST].name = "West";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_NORTH].name = "North";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_LB].name = "LB";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_RB].name = "RB";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_BACK].name = "Back";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_START].name = "Start";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_GUIDE].name = "Guide";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_L_THUMB].name = "L Thumb";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_R_THUMB].name = "R Thumb";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_DPAD_UP].name = "DPad Up";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_DPAD_RIGHT].name = "DPad Right";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_DPAD_DOWN].name = "DPad Down";
    ctrl->prv.button_channels_backing[LVKW_CTRL_BUTTON_DPAD_LEFT].name = "DPad Left";
    ctrl->pub.button_channels = ctrl->prv.button_channels_backing;
  }

  unsigned long ff_bits[FF_MAX / (8 * sizeof(unsigned long)) + 1] = {0};
  if (ioctl(dev->fd, EVIOCGBIT(EV_FF, sizeof(ff_bits)), ff_bits) >= 0) {
    bool has_rumble = ff_bits[FF_RUMBLE / (8 * sizeof(unsigned long))] &
                      (1UL << (FF_RUMBLE % (8 * sizeof(unsigned long))));
    if (has_rumble) {
      ctrl->pub.haptic_count = 2;
      ctrl->prv.haptic_channels_backing =
          lvkw_context_alloc(ctx_base, sizeof(LVKW_HapticChannelInfo) * ctrl->pub.haptic_count);
      if (ctrl->prv.haptic_channels_backing) {
        ctrl->prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_LOW_FREQ].name = "Low Frequency";
        ctrl->prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_HIGH_FREQ].name = "High Frequency";
        ctrl->pub.haptic_channels = ctrl->prv.haptic_channels_backing;
      }
    }
  }

  ctrl->prv.ctx_base = ctx_base;
  ctrl->prv.id = dev->id;
  ctrl->prv.next = ctx_base->prv.controller_list;
  ctx_base->prv.controller_list = ctrl;
  return ctrl;
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

  struct LVKW_CtrlDevice_Linux *dev = lvkw_context_alloc(ctx_base,
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
  dev->path = lvkw_context_alloc(ctx_base, path_len);
  if (dev->path) {
    memcpy(dev->path, path, path_len);
  }

  char name[256] = "Unknown Gamepad";
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    strncpy(name, "Unknown Gamepad", sizeof(name));
  }

  size_t name_len = strlen(name) + 1;
  dev->name = lvkw_context_alloc(ctx_base, name_len);
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
  dev->controller = _alloc_controller(ctx_base, dev);
  if (!dev->controller) {
    _remove_device(ctx_base, ctrl_ctx, dev, NULL);
    return;
  }

  if (ctrl_ctx->push_event) {
    LVKW_Event evt = {.controller_connection = {
                          .controller_ref = (LVKW_ControllerRef *)&dev->controller->pub,
                          .connected = true,
                      }};
    ctrl_ctx->push_event(LVKW_EVENT_TYPE_CONTROLLER_CONNECTION, NULL, &evt, ctrl_ctx->push_event_userdata);
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
                          .controller_ref =
                              dev->controller ? (LVKW_ControllerRef *)&dev->controller->pub : NULL,
                          .connected = false,
                      }};
    ctrl_ctx->push_event(LVKW_EVENT_TYPE_CONTROLLER_CONNECTION, NULL, &evt, ctrl_ctx->push_event_userdata);
  }

  if (dev->controller) {
    dev->controller->pub.flags |= LVKW_CONTROLLER_STATE_LOST;
  }

  if (dev->rumble_effect_id >= 0) {
    ioctl(dev->fd, EVIOCRMFF, dev->rumble_effect_id);
  }

  close(dev->fd);
  if (dev->name) {
    lvkw_context_free(ctx_base, dev->name);
  }
  if (dev->path) {
    lvkw_context_free(ctx_base, dev->path);
  }
  lvkw_context_free(ctx_base, dev);
}

void _lvkw_ctrl_init_context_Linux(LVKW_Context_Base *ctx_base,
                                   LVKW_ControllerContext_Linux *ctrl_ctx,
                                   LVKW_EventCallback push_event,
                                   void *push_event_userdata) {
  memset(ctrl_ctx, 0, sizeof(*ctrl_ctx));
  ctrl_ctx->push_event = push_event;
  ctrl_ctx->push_event_userdata = push_event_userdata;

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
      lvkw_context_free(ctx_base, curr->name);
    }
    if (curr->path) {
      lvkw_context_free(ctx_base, curr->path);
    }
    lvkw_context_free(ctx_base, curr);
    curr = next;
  }
}

static bool _process_device_events(struct LVKW_CtrlDevice_Linux *dev) {
  if (!dev->controller) return true;
  LVKW_Controller_Base *ctrl = dev->controller;
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
        ctrl->prv.buttons_backing[btn_idx] =
            (ev.value != 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
      }
    }
    else if (ev.type == EV_ABS) {
      int axis_idx = -1;
      float val = (LVKW_Scalar)0.0;
      struct input_absinfo abs;
      if (ioctl(dev->fd, (unsigned int)EVIOCGABS(ev.code), &abs) == 0) {
        float range = (float)(abs.maximum - abs.minimum);
        if (range != (LVKW_Scalar)0.0) {
          val = (float)(ev.value - abs.minimum) / range;
          if (ev.code == ABS_X || ev.code == ABS_Y || ev.code == ABS_RX || ev.code == ABS_RY) {
            val = val * (LVKW_Scalar)2.0 - (LVKW_Scalar)1.0;
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
          ctrl->prv.buttons_backing[LVKW_CTRL_BUTTON_DPAD_LEFT] =
              (ev.value < 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          ctrl->prv.buttons_backing[LVKW_CTRL_BUTTON_DPAD_RIGHT] =
              (ev.value > 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          break;
        case ABS_HAT0Y:
          ctrl->prv.buttons_backing[LVKW_CTRL_BUTTON_DPAD_UP] =
              (ev.value < 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          ctrl->prv.buttons_backing[LVKW_CTRL_BUTTON_DPAD_DOWN] =
              (ev.value > 0) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          break;
      }
      if (axis_idx >= 0 && axis_idx < LVKW_CTRL_ANALOG_STANDARD_COUNT) {
        ctrl->prv.analogs_backing[axis_idx].value = val;
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

int _lvkw_ctrl_get_poll_fds_Linux(LVKW_ControllerContext_Linux *ctrl_ctx, struct pollfd *pfds,
                                  int max_count) {
  int count = 0;
  if (count < max_count && ctrl_ctx->inotify_fd >= 0) {
    pfds[count].fd = ctrl_ctx->inotify_fd;
    pfds[count].events = POLLIN;
    count++;
  }

  struct LVKW_CtrlDevice_Linux *dev = ctrl_ctx->devices;
  while (dev && count < max_count) {
    pfds[count].fd = dev->fd;
    pfds[count].events = POLLIN;
    count++;
    dev = dev->next;
  }
  return count;
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
                                            uint32_t count, const LVKW_Scalar *intensities) {
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

  LVKW_Scalar low = (LVKW_Scalar)0.0;
  LVKW_Scalar high = (LVKW_Scalar)0.0;
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
              .strong_magnitude = (uint16_t)(low * (LVKW_Scalar)65535.0),
              .weak_magnitude = (uint16_t)(high * (LVKW_Scalar)65535.0),
          },
      .replay =
          {
              .length = 0,
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

LVKW_Status lvkw_ctrl_list_Linux(LVKW_Context *ctx_handle, LVKW_ControllerRef **out_refs,
                                 uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx_handle, out_refs, out_count);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  if (!out_refs) {
    uint32_t count = 0;
    for (LVKW_Controller_Base *c = ctx_base->prv.controller_list; c; c = c->prv.next) {
      if (c->pub.flags & LVKW_CONTROLLER_STATE_LOST) continue;
      count++;
    }
    *out_count = count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *out_count;
  uint32_t filled = 0;
  for (LVKW_Controller_Base *c = ctx_base->prv.controller_list; c && filled < room; c = c->prv.next) {
    if (c->pub.flags & LVKW_CONTROLLER_STATE_LOST) continue;
    out_refs[filled++] = (LVKW_ControllerRef *)&c->pub;
  }
  *out_count = filled;
  return LVKW_SUCCESS;
}

#endif
