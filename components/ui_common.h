#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "lvgl.h"

const char* ui_lang(const char* zh, const char* en);

// 通用组件
lv_obj_t* ui_create_back_btn(lv_obj_t *parent, lv_event_cb_t cb);
lv_obj_t* ui_create_title(lv_obj_t *parent, const char *text);

#endif