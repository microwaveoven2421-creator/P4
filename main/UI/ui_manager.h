#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "lvgl.h"

typedef enum {
    UI_MENU_HOME = 0,
    UI_MENU_SETTING,
    UI_MENU_FUEL,
    UI_MENU_FUEL_MEASUREMENT,
    UI_MENU_PRESSURE,
    UI_MENU_TEMPERATURE,
    UI_MENU_SYSTEM_INFO,
    UI_MENU_CALIBRATION,
    UI_MENU_MAX
} ui_menu_t;

void ui_manager_init(void);
void ui_menu_navigate(ui_menu_t menu);
void ui_menu_back(void);

#endif
