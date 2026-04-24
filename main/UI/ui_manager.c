#include "ui_manager.h"
#include "screens/ui_calibration.h"
#include "screens/ui_fuel.h"
#include "screens/ui_fuel_measurement.h"
#include "screens/ui_home.h"
#include "screens/ui_manufacturer_info.h"
#include "screens/ui_measurement_unit.h"
#include "screens/ui_pressure.h"
#include "screens/ui_service_record.h"
#include "screens/ui_setting.h"
#include "screens/ui_system_info.h"
#include "screens/ui_temperature.h"

#include <stdint.h>

static ui_menu_t history[16];
static uint8_t history_top = 0;
static lv_obj_t *current_screen = NULL;
static ui_menu_t current_menu = UI_MENU_HOME;

static lv_obj_t *create_menu(ui_menu_t menu)
{
    switch(menu) {
        case UI_MENU_HOME:
            return ui_home_create();
        case UI_MENU_SETTING:
            return ui_setting_create();
        case UI_MENU_FUEL:
            return ui_fuel_create();
        case UI_MENU_FUEL_MEASUREMENT:
            return ui_fuel_measurement_create();
        case UI_MENU_PRESSURE:
            return ui_pressure_create();
        case UI_MENU_TEMPERATURE:
            return ui_temperature_create();
        case UI_MENU_MEASUREMENT_UNIT:
            return ui_measurement_unit_create();
        case UI_MENU_SERVICE_RECORD:
            return ui_service_record_create();
        case UI_MENU_MANUFACTURER_INFO:
            return ui_manufacturer_info_create();
        case UI_MENU_CALIBRATION:
            return ui_calibration_create();
        case UI_MENU_SYSTEM_INFO:
            return ui_system_info_create();
        default:
            return NULL;
    }
}

static void load_menu(ui_menu_t menu)
{
    if(current_screen) {
        lv_obj_del(current_screen);
    }

    current_menu = menu;
    current_screen = create_menu(menu);

    if(current_screen) {
        lv_screen_load(current_screen);
    }
}

void ui_menu_navigate(ui_menu_t menu)
{
    if(menu >= UI_MENU_MAX) {
        return;
    }

    if(menu == current_menu) {
        history[history_top] = menu;
        load_menu(menu);
        return;
    }

    if(history_top < (uint8_t)(sizeof(history) / sizeof(history[0]) - 1U)) {
        history[++history_top] = menu;
    }
    else {
        for(uint8_t i = 1; i < (uint8_t)(sizeof(history) / sizeof(history[0])); i++) {
            history[i - 1U] = history[i];
        }

        history[history_top] = menu;
    }

    load_menu(menu);
}

void ui_menu_back(void)
{
    if(history_top == 0) {
        return;
    }

    history_top--;
    load_menu(history[history_top]);
}

void ui_menu_home(void)
{
    history[0] = UI_MENU_HOME;
    history_top = 0;
    load_menu(UI_MENU_HOME);
}

void ui_menu_refresh(void)
{
    load_menu(current_menu);
}

void ui_manager_init(void)
{
    history[0] = UI_MENU_HOME;
    history_top = 0;
    current_menu = UI_MENU_HOME;
    load_menu(UI_MENU_HOME);
}
