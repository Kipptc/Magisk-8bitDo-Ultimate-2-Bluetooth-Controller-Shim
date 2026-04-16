#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef BTN_GAMEPAD
#define BTN_GAMEPAD 0x130
#endif

#define TARGET_NAME "Nintendo Switch Pro Controller"
#define CHECK_INTERVAL_SECONDS 30

static int emit(int fd, int type, int code, int val) {
    struct input_event ie;
    memset(&ie, 0, sizeof(ie));
    ie.type = type;
    ie.code = code;
    ie.value = val;
    return write(fd, &ie, sizeof(ie));
}

static void setup_abs(int ufd, unsigned short code, int min, int max) {
    struct uinput_abs_setup s;
    memset(&s, 0, sizeof(s));
    s.code = code;
    s.absinfo.minimum = min;
    s.absinfo.maximum = max;
    s.absinfo.fuzz = 0;
    s.absinfo.flat = 0;
    s.absinfo.resolution = 0;
    if (ioctl(ufd, UI_ABS_SETUP, &s) < 0) {
        perror("UI_ABS_SETUP");
        exit(1);
    }
}

static int find_input_device_by_name(const char *target_name, char *out_path, size_t out_size) {
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        perror("opendir /dev/input");
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0) {
            continue;
        }

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);

        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            continue;
        }

        char name[256];
        memset(name, 0, sizeof(name));
        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
            if (strcmp(name, target_name) == 0) {
                close(fd);
                closedir(dir);
                snprintf(out_path, out_size, "%s", path);
                return 0;
            }
        }

        close(fd);
    }

    closedir(dir);
    return -1;
}

static int create_virtual_device(void) {
    int ufd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (ufd < 0) {
        perror("open /dev/uinput");
        return -1;
    }

    if (ioctl(ufd, UI_SET_EVBIT, EV_KEY) < 0) perror("UI_SET_EVBIT EV_KEY");
    if (ioctl(ufd, UI_SET_EVBIT, EV_ABS) < 0) perror("UI_SET_EVBIT EV_ABS");

    int keys[] = {
        BTN_EAST,
        BTN_WEST,
        BTN_NORTH,
        BTN_GAMEPAD,
        BTN_TL,
        BTN_TR,
        BTN_TL2,
        BTN_TR2,
        BTN_SELECT,
        BTN_START,
        BTN_THUMBL,
        BTN_THUMBR
    };

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        if (ioctl(ufd, UI_SET_KEYBIT, keys[i]) < 0) {
            perror("UI_SET_KEYBIT");
        }
    }

    int abs_codes[] = {
        ABS_X,
        ABS_Y,
        ABS_Z,
        ABS_RZ,
        ABS_HAT0X,
        ABS_HAT0Y,
        ABS_BRAKE,
        ABS_GAS
    };

    for (size_t i = 0; i < sizeof(abs_codes) / sizeof(abs_codes[0]); i++) {
        if (ioctl(ufd, UI_SET_ABSBIT, abs_codes[i]) < 0) {
            perror("UI_SET_ABSBIT");
        }
    }

    setup_abs(ufd, ABS_X, -32768, 32767);
    setup_abs(ufd, ABS_Y, -32768, 32767);
    setup_abs(ufd, ABS_Z, -32768, 32767);
    setup_abs(ufd, ABS_RZ, -32768, 32767);
    setup_abs(ufd, ABS_HAT0X, -1, 1);
    setup_abs(ufd, ABS_HAT0Y, -1, 1);
    setup_abs(ufd, ABS_BRAKE, 0, 255);
    setup_abs(ufd, ABS_GAS, 0, 255);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "8BitDo Trigger Shim");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x2022;
    usetup.id.product = 0x3001;
    usetup.id.version = 1;

    if (ioctl(ufd, UI_DEV_SETUP, &usetup) < 0) {
        perror("UI_DEV_SETUP");
        close(ufd);
        return -1;
    }

    if (ioctl(ufd, UI_DEV_CREATE) < 0) {
        perror("UI_DEV_CREATE");
        close(ufd);
        return -1;
    }

    sleep(1);
    return ufd;
}

static void destroy_virtual_device(int ufd) {
    if (ufd >= 0) {
        ioctl(ufd, UI_DEV_DESTROY);
        close(ufd);
    }
}

static void forward_event(int ufd, const struct input_event *ev) {
    if (ev->type == EV_KEY) {
        switch (ev->code) {
            case BTN_EAST:
                emit(ufd, EV_KEY, BTN_GAMEPAD, ev->value);
                break;
            case BTN_GAMEPAD:
                emit(ufd, EV_KEY, BTN_EAST, ev->value);
                break;
            case BTN_WEST:
                emit(ufd, EV_KEY, BTN_NORTH, ev->value);
                break;
            case BTN_NORTH:
                emit(ufd, EV_KEY, BTN_WEST, ev->value);
                break;
            case BTN_TL:
                emit(ufd, EV_KEY, BTN_TL, ev->value);
                break;
            case BTN_TR:
                emit(ufd, EV_KEY, BTN_TR, ev->value);
                break;
            case BTN_SELECT:
                emit(ufd, EV_KEY, BTN_SELECT, ev->value);
                break;
            case BTN_START:
                emit(ufd, EV_KEY, BTN_START, ev->value);
                break;
            case BTN_THUMBL:
                emit(ufd, EV_KEY, BTN_THUMBL, ev->value);
                break;
            case BTN_THUMBR:
                emit(ufd, EV_KEY, BTN_THUMBR, ev->value);
                break;
            case BTN_TL2:
                emit(ufd, EV_KEY, BTN_TL2, ev->value);
                emit(ufd, EV_ABS, ABS_BRAKE, ev->value ? 255 : 0);
                break;
            case BTN_TR2:
                emit(ufd, EV_KEY, BTN_TR2, ev->value);
                emit(ufd, EV_ABS, ABS_GAS, ev->value ? 255 : 0);
                break;
            default:
                break;
        }
    } else if (ev->type == EV_ABS) {
        switch (ev->code) {
            case ABS_X:
                emit(ufd, EV_ABS, ABS_X, ev->value);
                break;
            case ABS_Y:
                emit(ufd, EV_ABS, ABS_Y, ev->value);
                break;
            case ABS_Z:
                emit(ufd, EV_ABS, ABS_Z, ev->value);
                break;
            case ABS_RZ:
                emit(ufd, EV_ABS, ABS_RZ, ev->value);
                break;
            case ABS_HAT0X:
                emit(ufd, EV_ABS, ABS_HAT0X, ev->value);
                break;
            case ABS_HAT0Y:
                emit(ufd, EV_ABS, ABS_HAT0Y, ev->value);
                break;
            default:
                break;
        }
    } else if (ev->type == EV_SYN) {
        emit(ufd, EV_SYN, SYN_REPORT, 0);
    }
}

int main(void) {
    while (1) {
        char src[256];
        if (find_input_device_by_name(TARGET_NAME, src, sizeof(src)) < 0) {
            sleep(CHECK_INTERVAL_SECONDS);
            continue;
        }

        int infd = open(src, O_RDONLY);
        if (infd < 0) {
            sleep(CHECK_INTERVAL_SECONDS);
            continue;
        }

        if (ioctl(infd, EVIOCGRAB, 1) < 0) {
            perror("EVIOCGRAB");
        }

        int ufd = create_virtual_device();
        if (ufd < 0) {
            ioctl(infd, EVIOCGRAB, 0);
            close(infd);
            sleep(CHECK_INTERVAL_SECONDS);
            continue;
        }

        struct input_event ev;
        ssize_t n;
        while ((n = read(infd, &ev, sizeof(ev))) == sizeof(ev)) {
            forward_event(ufd, &ev);
        }

        destroy_virtual_device(ufd);
        ioctl(infd, EVIOCGRAB, 0);
        close(infd);

        sleep(CHECK_INTERVAL_SECONDS);
    }

    return 0;
}