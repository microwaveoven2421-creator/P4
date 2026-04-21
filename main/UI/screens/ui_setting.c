#include "screens/ui_setting.h"
#include "ui_common.h"
#include "UI/ui.h"
#include "UI/ui_manager.h"
#include "bsp/display.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *setting_items[][2] = {
    {"Date/Time", "Date & Time"},
    {"LCD Brightness", "LCD Brightness"},
    {"Bluetooth", "Bluetooth"},
    {"Region", "Region"},
    {"Language", "Language"},
    {"GPS", "GPS"},
    {"Wi-Fi", "WIFI"},
    {"Key Sound", "Key Sound"},
    {"Unit", "Unit"},
    {"Reset", "Reset"},
    {"Info", "Info"},
};

static char setting_values[16][16] = {
    "", "50", "ON", "CN", "ZH",
    "OFF", "OFF", "ON", "Metric", "", ""
};

static lv_obj_t *value_labels[16];

static void setting_event(lv_event_t *e)
{
    uint32_t i = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    switch (i) {
        case 1:
        {
            static int val = 50;
            val += 10;
            if (val > 100) {
                val = 10;
            }

            snprintf(setting_values[1], sizeof(setting_values[1]), "%d", val);
            lv_label_set_text(value_labels[1], setting_values[1]);
            bsp_display_brightness_set(val);
            break;
        }

        case 2:
        case 5:
        case 6:
        case 7:
            if (strcmp(setting_values[i], "ON") == 0) {
                strcpy(setting_values[i], "OFF");
            } else {
                strcpy(setting_values[i], "ON");
            }

            lv_label_set_text(value_labels[i], setting_values[i]);
            break;

        case 4:
            if (ui_get_language() == LANG_ZH) {
                ui_set_language(LANG_EN);
                strcpy(setting_values[4], "EN");
            } else {
                ui_set_language(LANG_ZH);
                strcpy(setting_values[4], "ZH");
            }

            ui_menu_navigate(UI_MENU_SETTING);
            break;

        default:
            break;
    }
}

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t* ui_setting_create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);

    ui_create_title(screen, ui_lang("Setting", "Settings"));
    ui_create_back_btn(screen, back_event);

    lv_obj_t *list = lv_obj_create(screen);
    lv_obj_set_size(list, 440, 650);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < 11; i++) {
        lv_obj_t *btn = lv_button_create(list);
        lv_obj_set_size(btn, 440, 60);
        lv_obj_add_event_cb(btn, setting_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, ui_lang(setting_items[i][0], setting_items[i][1]));
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, 0);

        value_labels[i] = lv_label_create(btn);
        lv_label_set_text(value_labels[i], setting_values[i]);
        lv_obj_align(value_labels[i], LV_ALIGN_RIGHT_MID, -20, 0);
    }

    return screen;
}
