#ifndef UI_POPUP_DATETIME_H
#define UI_POPUP_DATETIME_H

#include "lvgl.h"

typedef void (*datetime_confirm_cb_t)(int year, int month, int day,
                                      int hour, int minute);

/**
 * 初始化弹窗（只调用一次）
 */
void ui_popup_datetime_init(lv_obj_t *parent);

/**
 * 显示弹窗
 */
void ui_popup_datetime_show(void);

/**
 * 隐藏弹窗
 */
void ui_popup_datetime_hide(void);

/**
 * 设置确认回调
 */
void ui_popup_datetime_set_confirm_cb(datetime_confirm_cb_t cb);

#endif