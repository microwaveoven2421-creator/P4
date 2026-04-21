#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "lvgl.h"

typedef void (*ui_input_cb_t)(const char *text, void *user_data);

typedef enum {
    UI_INPUT_MODE_NUMBER = 0,
    UI_INPUT_MODE_FULL,
} ui_input_mode_t;

void ui_input_init(lv_obj_t *parent);
void ui_input_show(const char *title,
                   const char *init_text,
                   ui_input_cb_t cb,
                   void *user_data);
void ui_input_show_mode(const char *title,
                        const char *init_text,
                        ui_input_mode_t mode,
                        ui_input_cb_t cb,
                        void *user_data);
void ui_input_hide(void);

#endif
