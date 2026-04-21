#include "ui_date_picker.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>

static lv_obj_t *popup;
static lv_obj_t *panel;
static lv_obj_t *calendar;
static lv_calendar_date_t selected_date;
static ui_date_picker_cb_t confirm_cb;
static void *confirm_user_data;

static void calendar_event_cb(lv_event_t *e)
{
    lv_calendar_date_t date;

    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }

    if(lv_calendar_get_pressed_date(calendar, &date)) {
        selected_date = date;
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

void ui_date_picker_init(lv_obj_t *parent)
{
    lv_obj_t *host = parent ? parent : lv_layer_top();
    lv_obj_t *btn;
    lv_obj_t *label;

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

    label = lv_label_create(panel);
    lv_label_set_text(label, "选择日期");
    ui_apply_btn_text_style(label);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 4);

    calendar = lv_calendar_create(panel);
    lv_obj_set_size(calendar, 428, 360);
    lv_obj_align(calendar, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_add_header_dropdown(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_add_header_arrow(calendar);
#endif

    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 160, 52);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEEF3F8), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_add_event_cb(btn, action_event_cb, LV_EVENT_CLICKED, (void *)0);
    label = lv_label_create(btn);
    lv_label_set_text(label, "取消");
    ui_apply_btn_text_style(label);
    lv_obj_center(label);

    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 160, 52);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCE8F6), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x93A9C2), 0);
    lv_obj_add_event_cb(btn, action_event_cb, LV_EVENT_CLICKED, (void *)1);
    label = lv_label_create(btn);
    lv_label_set_text(label, "确定");
    ui_apply_btn_text_style(label);
    lv_obj_center(label);
}

void ui_date_picker_show(int year, int month, int day, ui_date_picker_cb_t cb, void *user_data)
{
    if(!popup) {
        ui_date_picker_init(lv_layer_top());
    }

    selected_date.year = year;
    selected_date.month = month;
    selected_date.day = day;
    confirm_cb = cb;
    confirm_user_data = user_data;

    lv_calendar_set_today_date(calendar, year, month, day);
    lv_calendar_set_month_shown(calendar, year, month);
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
