#include "ui_popup_datetime.h"
#include <string.h>

/* ================== 静态对象 ================== */
static lv_obj_t *popup;
static lv_obj_t *calendar;
static lv_obj_t *spin_hour;
static lv_obj_t *spin_min;

static datetime_confirm_cb_t confirm_cb = NULL;

/* ================== 按钮事件 ================== */
static void btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));

    /* ===== 点击 OK ===== */
    if(strcmp(txt, "OK") == 0)
    {
        lv_calendar_date_t date;

        /* 获取用户选中的日期 */
        if(lv_calendar_get_pressed_date(calendar, &date) == LV_RES_OK)
        {
            int hour = lv_spinbox_get_value(spin_hour);
            int minute = lv_spinbox_get_value(spin_min);

            if(confirm_cb)
            {
                confirm_cb(date.year, date.month, date.day, hour, minute);
            }
        }
    }

    /* ===== 关闭弹窗 ===== */
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
}

/* ================== 初始化 ================== */
void ui_popup_datetime_init(lv_obj_t *parent)
{
    popup = lv_obj_create(parent);
    lv_obj_set_size(popup, 300, 320);
    lv_obj_center(popup);

    lv_obj_set_style_radius(popup, 10, 0);
    lv_obj_set_style_bg_color(popup, lv_color_white(), 0);

    /* 初始隐藏 */
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);

    /* ===== 日历 ===== */
    calendar = lv_calendar_create(popup);
    lv_obj_set_size(calendar, 280, 180);
    lv_obj_align(calendar, LV_ALIGN_TOP_MID, 0, 5);

    /* 默认显示今天（更符合用户习惯） */
    lv_calendar_date_t today = {2024, 1, 1};  // 你可以后面接RTC
    lv_calendar_set_today_date(calendar, today.year, today.month, today.day);
    lv_calendar_set_showed_date(calendar, today.year, today.month);

    /* ===== 时间选择 ===== */
    spin_hour = lv_spinbox_create(popup);
    lv_spinbox_set_range(spin_hour, 0, 23);
    lv_spinbox_set_digit_format(spin_hour, 2, 0);
    lv_obj_set_width(spin_hour, 60);
    lv_obj_align(spin_hour, LV_ALIGN_LEFT_MID, 40, 20);

    spin_min = lv_spinbox_create(popup);
    lv_spinbox_set_range(spin_min, 0, 59);
    lv_spinbox_set_digit_format(spin_min, 2, 0);
    lv_obj_set_width(spin_min, 60);
    lv_obj_align(spin_min, LV_ALIGN_RIGHT_MID, -40, 20);

    /* ===== OK按钮 ===== */
    lv_obj_t *btn_ok = lv_btn_create(popup);
    lv_obj_set_size(btn_ok, 80, 40);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    lv_obj_add_event_cb(btn_ok, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_ok = lv_label_create(btn_ok);
    lv_label_set_text(label_ok, "OK");
    lv_obj_center(label_ok);

    /* ===== Cancel按钮 ===== */
    lv_obj_t *btn_cancel = lv_btn_create(popup);
    lv_obj_set_size(btn_cancel, 80, 40);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_add_event_cb(btn_cancel, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, "Cancel");
    lv_obj_center(label_cancel);
}

/* ================== 显示 ================== */
void ui_popup_datetime_show(void)
{
    if(popup)
    {
        lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ================== 隐藏 ================== */
void ui_popup_datetime_hide(void)
{
    if(popup)
    {
        lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ================== 设置回调 ================== */
void ui_popup_datetime_set_confirm_cb(datetime_confirm_cb_t cb)
{
    confirm_cb = cb;
}