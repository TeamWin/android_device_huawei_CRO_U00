#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cutils/klog.h>

#include <healthd.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define LOGE(x...)                                                             \
    do {                                                                       \
        KLOG_ERROR("charger", x);                                              \
    } while (0)
#define LOGV(x...)                                                             \
    do {                                                                       \
        KLOG_DEBUG("charger", x);                                              \
    } while (0)

#define RED_LED_PATH "/sys/class/leds/red/brightness"
#define GREEN_LED_PATH "/sys/class/leds/green/brightness"

// Import data from system/core/healthd/healthd_mode_charger.cpp

enum {
    RED_LED   = 0x01 << 0,
    GREEN_LED = 0x01 << 1,
};

struct led_ctl {
    int color;
    const char *path;
};

static struct led_ctl leds[2] = {
    { RED_LED, RED_LED_PATH },
    { GREEN_LED, GREEN_LED_PATH },
};

struct soc_led_color_mapping {
    int soc;
    int color;
};

static struct soc_led_color_mapping soc_leds[3] = {
    { 15, RED_LED },
    { 90, RED_LED | GREEN_LED },
    { 100, GREEN_LED },
};

static void
set_tricolor_led(int on, int color)
{
    int fd, i;
    char buffer[10];

    for (i = 0; i < (int)ARRAY_SIZE(leds); i++) {
        if ((color & leds[i].color) &&
            (access(leds[i].path, R_OK | W_OK) == 0)) {
            fd = open(leds[i].path, O_RDWR);
            if (fd < 0) {
                LOGE("Could not open led node %d\n", i);
                continue;
            }
            if (on)
                snprintf(buffer, sizeof(int), "%d\n", 255);
            else
                snprintf(buffer, sizeof(int), "%d\n", 0);

            if (write(fd, buffer, strlen(buffer)) < 0)
                LOGE("Could not write to led node\n");
            close(fd);
        }
    }
}

static void
set_battery_soc_leds(int soc, bool charger_conn)
{
    int i, color;
    static int old_color = 0;

    if (!charger_conn) {
        if (old_color) {
            set_tricolor_led(0, old_color);
            old_color = 0;
        }
        return;
    }

    for (i = 0; i < (int)ARRAY_SIZE(soc_leds); i++) {
        if (soc <= soc_leds[i].soc)
            break;
    }

    color = soc_leds[i].color;
    if (old_color != color) {
        set_tricolor_led(0, old_color);
        set_tricolor_led(1, color);
        old_color = color;
        // LOGV("soc = %d, set led color 0x%x\n", soc, color);
    }
}

void
healthd_board_init(struct healthd_config *)
{
    // use defaults
}

int
healthd_board_battery_update(struct android::BatteryProperties *props)
{
    if (props) {
        bool charger_connected = props->chargerAcOnline ||
                                 props->chargerUsbOnline ||
                                 props->chargerWirelessOnline;

        set_battery_soc_leds(props->batteryLevel, charger_connected);
    }

    return -1; // without dmesg
}
