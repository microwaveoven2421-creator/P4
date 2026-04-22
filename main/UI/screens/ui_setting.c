#include "screens/ui_setting.h"
#include "ui_common.h"
#include "UI/ui.h"
#include "UI/ui_manager.h"
#include "bsp/display.h"
#include "comp/ui_input.h"
#include "comp/ui_date_picker.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *setting_items[][2] = {
    {"日期/时间", "Date & Time"},
    {"LCD亮度", "LCD Brightness"},
    {"蓝牙", "Bluetooth"},
    {"地区", "Region"},
    {"语言", "Language"},
    {"GPS", "GPS"},
    {"Wi-Fi", "Wi-Fi"},
    {"按键音", "Key Sound"},
    {"单位", "Unit"},
    {"恢复默认", "Reset"},
    {"系统信息", "Info"},
};

static char setting_values[16][32] = {
    "", "50", "ON", "CN", "ZH",
    "OFF", "OFF", "ON", "Metric", "", ""
};

static lv_obj_t *value_labels[16];
static lv_timer_t *clock_timer;
static time_t base_epoch;
static uint32_t base_tick_ms;
static bool datetime_ready;

static time_t current_epoch(void)
{
    return base_epoch + (time_t)(lv_tick_elaps(base_tick_ms) / 1000U);
}

static void update_datetime_text(void)
{
    struct tm *now_tm;
    time_t now = current_epoch();

    now_tm = localtime(&now);
    if(!now_tm) {
        return;
    }

    strftime(setting_values[0], sizeof(setting_values[0]), "%Y-%m-%d %H:%M", now_tm);
    if(value_labels[0]) {
        lv_label_set_text(value_labels[0], setting_values[0]);
    }
}

static void init_datetime_if_needed(void)
{
    struct tm tm_info = {0};

    if(datetime_ready) {
        return;
    }

    tm_info.tm_year = 2026 - 1900;
    tm_info.tm_mon = 4 - 1;
    tm_info.tm_mday = 21;
    tm_info.tm_hour = 12;
    tm_info.tm_min = 0;
    tm_info.tm_sec = 0;
    base_epoch = mktime(&tm_info);
    base_tick_ms = lv_tick_get();
    datetime_ready = true;
}

static void clock_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    update_datetime_text();
}

static void date_selected_cb(int year, int month, int day, void *user_data)
{
    struct tm *now_tm;
    struct tm tm_info;
    time_t now;

    (void)user_data;

    now = current_epoch();
    now_tm = localtime(&now);
    if(!now_tm) {
        return;
    }

    tm_info = *now_tm;
    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = day;
    base_epoch = mktime(&tm_info);
    base_tick_ms = lv_tick_get();
    update_datetime_text();
}

static void input_cb(const char *text, void *user_data)
{
    int i = (int)(uintptr_t)user_data;

    snprintf(setting_values[i], sizeof(setting_values[i]), "%s", text ? text : "");
    lv_label_set_text(value_labels[i], setting_values[i]);
}

static void open_date_picker(void)
{
    struct tm *now_tm;
    time_t now = current_epoch();

    now_tm = localtime(&now);
    if(!now_tm) {
        return;
    }

    ui_date_picker_show(now_tm->tm_year + 1900,
                        now_tm->tm_mon + 1,
                        now_tm->tm_mday,
                        date_selected_cb,
                        NULL);
}

static void setting_event(lv_event_t *e)
{
    uint32_t i = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    switch(i) {
        case 0:
            open_date_picker();
            break;
        case 1:
        {
            static int val = 50;
            val += 10;
            if(val > 100) {
                val = 10;
            }

            snprintf(setting_values[1], sizeof(setting_values[1]), "%d", val);
            lv_label_set_text(value_labels[1], setting_values[1]);
            bsp_display_brightness_set(val);
            break;
        }
        case 3:
            ui_input_show_mode("输入地区", setting_values[3], UI_INPUT_MODE_FULL, input_cb, (void *)(uintptr_t)3);
            break;
        case 4:
            if(ui_get_language() == LANG_ZH) {
                ui_set_language(LANG_EN);
                snprintf(setting_values[4], sizeof(setting_values[4]), "EN");
            }
            else {
                ui_set_language(LANG_ZH);
                snprintf(setting_values[4], sizeof(setting_values[4]), "ZH");
            }

            ui_menu_navigate(UI_MENU_SETTING);
            break;
        case 10:
            ui_menu_navigate(UI_MENU_SYSTEM_INFO);
            break;
        case 2:
        case 5:
        case 6:
        case 7:
            if(strcmp(setting_values[i], "ON") == 0) {
                snprintf(setting_values[i], sizeof(setting_values[i]), "OFF");
            }
            else {
                snprintf(setting_values[i], sizeof(setting_values[i]), "ON");
            }

            lv_label_set_text(value_labels[i], setting_values[i]);
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

static void screen_delete_event(lv_event_t *e)
{
    (void)e;

    if(clock_timer) {
        lv_timer_delete(clock_timer);
        clock_timer = NULL;
    }
}

lv_obj_t* ui_setting_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    init_datetime_if_needed();

    ui_create_title(screen, ui_lang("设置", "Settings"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(int i = 0; i < 11; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_obj_add_event_cb(btn, setting_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_label_set_text(label, ui_lang(setting_items[i][0], setting_items[i][1]));
        ui_apply_btn_text_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        value_labels[i] = lv_label_create(btn);
        // ===== 防止时间过长遮挡 =====
        lv_obj_set_width(value_labels[i], 140);
        lv_label_set_long_mode(value_labels[i], LV_LABEL_LONG_SCROLL_CIRCULAR);

        lv_label_set_text(value_labels[i], setting_values[i]);
        ui_apply_btn_text_style(value_labels[i]);
        lv_obj_align(value_labels[i], LV_ALIGN_RIGHT_MID, 0, 0);// -10,0 是为了防止过长的值遮挡箭头
    }

    update_datetime_text();
    clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    lv_obj_add_event_cb(screen, screen_delete_event, LV_EVENT_DELETE, NULL);

    return screen;
}
