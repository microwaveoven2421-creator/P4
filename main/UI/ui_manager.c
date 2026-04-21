#include "ui_manager.h"
#include "screens/ui_home.h"
#include "screens/ui_setting.h"
#include "screens/ui_fuel.h"

#include <stdint.h>

static ui_menu_t history[8];
static uint8_t history_top = 0;

static lv_obj_t *current_screen = NULL;

static lv_obj_t* create_menu(ui_menu_t menu)
{
    switch (menu) {
        case UI_MENU_HOME:
            return ui_home_create();
        case UI_MENU_SETTING:
            return ui_setting_create();
        case UI_MENU_FUEL:
            return ui_fuel_create();
        default:
            return NULL;
    }
}

void ui_menu_navigate(ui_menu_t menu)
{
    if (menu >= UI_MENU_MAX) {
        return;
    }

    if (history_top < 7) {
        history[++history_top] = menu;
    }

    if (current_screen) {
        lv_obj_del(current_screen);
    }

    current_screen = create_menu(menu);
    lv_screen_load(current_screen);
}

void ui_menu_back(void)
{
    ui_menu_t prev;

    if (history_top == 0) {
        return;
    }

    history_top--;
    prev = history[history_top];

    if (current_screen) {
        lv_obj_del(current_screen);
    }

    current_screen = create_menu(prev);
    lv_screen_load(current_screen);
}

void ui_manager_init(void)
{
    history[0] = UI_MENU_HOME;
    history_top = 0;

    ui_menu_navigate(UI_MENU_HOME);
}
