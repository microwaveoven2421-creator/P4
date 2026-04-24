#ifndef UI_CONFIRM_H
#define UI_CONFIRM_H

#include "lvgl.h"

typedef void (*ui_confirm_cb_t)(void *user_data);

void ui_confirm_init(lv_obj_t *parent);

void ui_confirm_show(const char *text,
                     ui_confirm_cb_t ok_cb,
                     void *user_data);
void ui_confirm_show_ok(const char *text,
                        ui_confirm_cb_t ok_cb,
                        void *user_data);

void ui_confirm_hide(void);

#endif
