#include "screens/ui_setting.h"
#include "ui_common.h"
#include "UI/ui.h"
#include "UI/ui_manager.h"
#include "bsp/display.h"
#include "comp/ui_input.h"
#include "comp/ui_date_picker.h"
#include "comp/ui_confirm.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


//================== 字符串定义 =====================
#define ZH_RESTORE_DEFAULT_CONFIRM "\xE7\xA1\xAE\xE5\xAE\x9A\xE8\xA6\x81\xE6\x81\xA2\xE5\xA4\x8D\xE9\xBB\x98\xE8\xAE\xA4\xE8\xAE\xBE\xE7\xBD\xAE\xE5\x90\x97?"
#define ZH_RESTORE_DEFAULT_DONE "\xE5\xB7\xB2\xE6\x81\xA2\xE5\xA4\x8D\xE9\xBB\x98\xE8\xAE\xA4\xE8\xAE\xBE\xE7\xBD\xAE"
#define ZH_CANCEL "\xE5\x8F\x96\xE6\xB6\x88"
#define ZH_OK "\xE7\xA1\xAE\xE5\xAE\x9A"
//================== 变量定义 =====================
static lv_obj_t *value_labels[16];
static lv_timer_t *clock_timer;
static time_t base_epoch;
static uint32_t base_tick_ms;
static bool datetime_ready;
static int brightness_value = 50;
static void reset_confirm_cb(void *user_data);

//================== 设置项定义 =====================
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
    {"\xE8\xAE\xBE\xE5\xA4\x87\xE4\xBF\xA1\xE6\x81\xAF", "Device Info"},
};


static char setting_values[16][32] = {
    "", "50", "ON", "CN", "ZH",
    "OFF", "OFF", "ON", "Metric", "", ""
};

typedef enum {
    SETTING_DATETIME = 0,
    SETTING_BRIGHTNESS,
    SETTING_BLUETOOTH,
    SETTING_REGION,
    SETTING_LANGUAGE,
    SETTING_GPS,
    SETTING_WIFI,
    SETTING_KEY_SOUND,
    SETTING_UNIT,
    SETTING_RESET,
    SETTING_INFO
} setting_id_t;

static void update_language_value_text(void)
{
    if(ui_get_language() == LANG_ZH) {
        snprintf(setting_values[SETTING_LANGUAGE], sizeof(setting_values[SETTING_LANGUAGE]), "%s", "\xE4\xB8\xAD\xE6\x96\x87");
    }
    else {
        snprintf(setting_values[SETTING_LANGUAGE], sizeof(setting_values[SETTING_LANGUAGE]), "%s", "English");
    }

    if(value_labels[SETTING_LANGUAGE] && lv_obj_is_valid(value_labels[SETTING_LANGUAGE])) {
        lv_label_set_text(value_labels[SETTING_LANGUAGE], setting_values[SETTING_LANGUAGE]);
    }
}

//====================================================
//================== 事件callback函数 =====================
//====================================================

static void restore_default_confirm_cb(void *user_data)
{
    (void)user_data;

    reset_confirm_cb(NULL);
}

static void reset_confirm_cb(void *user_data)
{
    (void)user_data;

    // 恢复默认值
    strcpy(setting_values[1], "50");
    strcpy(setting_values[1], "50");
    brightness_value = 50;
    strcpy(setting_values[2], "ON");
    strcpy(setting_values[3], "CN");
    //strcpy(setting_values[4], "ZH");
    strcpy(setting_values[5], "OFF");
    strcpy(setting_values[6], "OFF");
    strcpy(setting_values[7], "ON");
    strcpy(setting_values[8], "Metric");
    update_language_value_text();

    // 刷新UI
    for(int i = 0; i < 9; i++) {
        lv_label_set_text(value_labels[i], setting_values[i]);
    }

    // 亮度同步
    bsp_display_brightness_set(50);
}

static time_t current_epoch(void)//获取当前时间的epoch，单位是秒
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

    strftime(setting_values[0], sizeof(setting_values[0]), "%Y-%m-%d", now_tm);
    if(value_labels[0] && lv_obj_is_valid(value_labels[0])) {
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

//日期选择完成回调，year、month、day是选择的年月日，user_data是之前传入的用户数据，这里不需要用到
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

//输入完成回调，text是输入的文本，user_data是之前传入的用户数据，这里用来区分是哪个设置项
static void input_cb(const char *text, void *user_data)
{
    int i = (int)(uintptr_t)user_data;

    snprintf(setting_values[i], sizeof(setting_values[i]), "%s", text ? text : "");
    lv_label_set_text(value_labels[i], setting_values[i]);
}

static void open_date_picker(void)//打开日期选择器，初始值为当前时间
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

//====================================================
//================== 事件函数 =====================
//====================================================
static void setting_event(lv_event_t *e)//设置项点击事件
{
    setting_id_t i = (setting_id_t)(uintptr_t)lv_event_get_user_data(e);

    switch(i) {
        case SETTING_DATETIME:
            open_date_picker();
            break;
        case SETTING_BRIGHTNESS://亮度设置，简单的增加10%，超过100%则回到10%
        {
            brightness_value += 10;
            if(brightness_value > 100) {
                brightness_value = 10;
            }

            snprintf(setting_values[1], sizeof(setting_values[1]), "%d", brightness_value);
            lv_label_set_text(value_labels[1], setting_values[1]);
            bsp_display_brightness_set(brightness_value);
            break;
        }
        case SETTING_REGION://地区设置，直接输入文本，没做复杂的地区选择，所以不区分语言了
        {
            ui_input_show_mode("输入地区", setting_values[i], UI_INPUT_MODE_FULL, input_cb, (void *)(uintptr_t)i);
            break;
        }
        case SETTING_LANGUAGE://语言切换
        {
            if(ui_get_language() == LANG_ZH) {
                ui_set_language(LANG_EN);
            }
            else {
                ui_set_language(LANG_ZH);
            }

            update_language_value_text();
            ui_menu_refresh();
            break;
        }
        //都是开关，直接切换状态即可，这个顺序不要改，因为和setting_values的索引相关
        case SETTING_BLUETOOTH://蓝牙
        case SETTING_GPS://GPS
        case SETTING_WIFI://Wi-Fi
        case SETTING_KEY_SOUND://按键音
        {
            if(strcmp(setting_values[i], "ON") == 0) {
                snprintf(setting_values[i], sizeof(setting_values[i]), "OFF");
            }
            else {
                snprintf(setting_values[i], sizeof(setting_values[i]), "ON");
            }

            lv_label_set_text(value_labels[i], setting_values[i]);
            break;
        }
        case SETTING_UNIT://单位
        {
            ui_menu_navigate(UI_MENU_MEASUREMENT_UNIT);
            break;
        }
        case SETTING_RESET://恢复默认
            {
                ui_confirm_show(ui_lang(ZH_RESTORE_DEFAULT_CONFIRM, "Restore default settings?"), reset_confirm_cb, NULL);
                break;
            }
        case SETTING_INFO://系统信息
        {
            ui_menu_navigate(UI_MENU_SYSTEM_INFO);
            break;
        }

        default:
            break;
    }
}

static void restore_default_event(lv_event_t *e)//恢复默认
{
    (void)e;

    ui_confirm_show(
        ui_lang(ZH_RESTORE_DEFAULT_CONFIRM, "Restore default settings?"),
        restore_default_confirm_cb,
        NULL
    );
}

static void back_event(lv_event_t *e)//返回
{
    (void)e;
    ui_menu_back();
}

static void screen_delete_event(lv_event_t *e)//界面被删除时的事件
{
    (void)e;

    if(clock_timer) {
        lv_timer_delete(clock_timer);
        clock_timer = NULL;
    }

    for(int i = 0; i < 16; i++) {
        value_labels[i] = NULL;
    }
}

//====================================================
//================== 主要界面创建函数 ===================
//====================================================
lv_obj_t* ui_setting_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    init_datetime_if_needed();
    update_language_value_text();

    ui_create_title(screen, ui_lang("设置", "Settings"));
    ui_create_nav_bar(screen);
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
        lv_label_set_long_mode(value_labels[i], LV_LABEL_LONG_DOT);

        lv_label_set_text(value_labels[i], setting_values[i]);
        ui_apply_btn_text_style(value_labels[i]);
        lv_obj_align(value_labels[i], LV_ALIGN_RIGHT_MID, 0, 0);//对齐到右侧中间
    }

    update_datetime_text();
    clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    lv_obj_add_event_cb(screen, screen_delete_event, LV_EVENT_DELETE, NULL);

    ui_confirm_init(screen);

    return screen;
}
