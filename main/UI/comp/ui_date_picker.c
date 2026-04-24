#include "ui_date_picker.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>

static lv_obj_t *popup;
static lv_obj_t *panel;
static lv_obj_t *calendar;
static lv_obj_t *title_label;
static lv_obj_t *cancel_label;
static lv_obj_t *ok_label;
static lv_calendar_date_t selected_date;
static ui_date_picker_cb_t confirm_cb;
static void *confirm_user_data;
static lv_calendar_date_t highlight[1];

static void calendar_event_cb(lv_event_t *e)
{
    lv_calendar_date_t date;

    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    if(lv_calendar_get_pressed_date(calendar, &date)) {
        selected_date = date;

        // ===== 高亮选中日期 =====
        lv_calendar_set_highlighted_dates(calendar, &selected_date, 1);
    }
}

static void action_event_cb(lv_event_t *e)
{
    bool confirm = (bool)(uintptr_t)lv_event_get_user_data(e);

    if(confirm && confirm_cb) {
        confirm_cb(selected_date.year, selected_date.month, selected_date.day, confirm_user_data);
    }

    ui_date_picker_hide();
}

static void update_language_text(void)
{
    lv_label_set_text(title_label, ui_lang("选择日期", "Select Date"));
    lv_label_set_text(cancel_label, ui_lang("取消", "Cancel"));
    lv_label_set_text(ok_label, ui_lang("确定", "OK"));
}

void ui_date_picker_init(lv_obj_t *parent)
{
    lv_obj_t *host = parent ? parent : lv_layer_top();
    lv_obj_t *btn;

    if(popup) {
        return;
    }

    popup = lv_obj_create(host);
    lv_obj_remove_style_all(popup);
    lv_obj_set_size(popup, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_30, 0);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(popup);
    lv_obj_set_size(panel, 460, 520);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 24, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFBFCFE), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(panel, 8, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_pad_all(panel, 16, 0);

    title_label = lv_label_create(panel);
    lv_label_set_text(title_label, ui_lang("选择日期", "Select Date"));
    ui_apply_btn_text_style(title_label);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 4);

    calendar = lv_calendar_create(panel);

    lv_obj_set_size(calendar, 428, 360);
    lv_obj_align(calendar, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
#if LV_USE_CALENDAR_HEADER_DROPDOWN

    lv_calendar_add_header_dropdown(calendar);

    static const char *year_list =
        "1996\n1997\n1998\n1999\n2000\n"
        "2001\n2002\n2003\n2004\n2005\n"
        "2006\n2007\n2008\n2009\n2010\n"
        "2011\n2012\n2013\n2014\n2015\n"
        "2016\n2017\n2018\n2019\n2020\n"
        "2021\n2022\n2023\n2024\n2025\n"
        "2026\n2027\n2028\n2029\n2030\n"
        "2031\n2032\n2033\n2034\n2035\n"
        "2036\n2037\n2038\n2039\n2040\n"
        "2041\n2042\n2043\n2044\n2045\n"
        "2046\n2047\n2048\n2049\n2050\n"
        "2051\n2052\n2053\n2054\n2055\n"
        "2056";

    lv_calendar_header_dropdown_set_year_list(calendar, year_list);

#endif

    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 160, 52);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEEF3F8), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_add_event_cb(btn, action_event_cb, LV_EVENT_CLICKED, (void *)0);
    cancel_label = lv_label_create(btn);
    lv_label_set_text(cancel_label, ui_lang("取消", "Cancel"));
    ui_apply_btn_text_style(cancel_label);
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_font(cancel_label, ui_font_32(), 0);

    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 160, 52);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCE8F6), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x93A9C2), 0);
    lv_obj_add_event_cb(btn, action_event_cb, LV_EVENT_CLICKED, (void *)1);
    ok_label = lv_label_create(btn);
    lv_label_set_text(ok_label, ui_lang("确定", "OK"));
    ui_apply_btn_text_style(ok_label);
    lv_obj_center(ok_label);
    lv_obj_set_style_text_font(ok_label, ui_font_32(), 0);
}

void ui_date_picker_show(int year, int month, int day, ui_date_picker_cb_t cb, void *user_data)
{
    if(!popup) {
        ui_date_picker_init(lv_layer_top());
    }

    update_language_text();

    selected_date.year = year;
    selected_date.month = month;
    selected_date.day = day;

    highlight[0] = selected_date;
    lv_calendar_set_highlighted_dates(calendar, highlight, 1);// 高亮选中日期 

    confirm_cb = cb;
    confirm_user_data = user_data;

    lv_calendar_set_today_date(calendar, year, month, day);// 设置今天日期为选中日期，避免用户选择的日期被置灰
    lv_calendar_set_showed_date(calendar, year, month);// 设置显示的月份为选中日期所在月份
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(popup);
}

void ui_date_picker_hide(void)
{
    if(!popup) {
        return;
    }

    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
}
