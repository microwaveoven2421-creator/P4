#ifndef UI_DATE_PICKER_H
#define UI_DATE_PICKER_H

#include "lvgl.h"

typedef void (*ui_date_picker_cb_t)(int year, int month, int day, void *user_data);

void ui_date_picker_init(lv_obj_t *parent);
void ui_date_picker_show(int year, int month, int day, ui_date_picker_cb_t cb, void *user_data);
void ui_date_picker_hide(void);

#endif
