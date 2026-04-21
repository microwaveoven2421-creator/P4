#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "lvgl.h"

const char* ui_lang(const char* zh, const char* en);

const lv_font_t* ui_font_32(void);
lv_obj_t* ui_create_screen(void);
lv_obj_t* ui_create_title(lv_obj_t *parent, const char *text);
lv_obj_t* ui_create_back_btn(lv_obj_t *parent, lv_event_cb_t cb);
lv_obj_t* ui_create_page_list(lv_obj_t *parent);
lv_obj_t* ui_create_list_btn(lv_obj_t *parent);
void ui_apply_btn_text_style(lv_obj_t *label);
void ui_apply_title_text_style(lv_obj_t *label);

#endif
