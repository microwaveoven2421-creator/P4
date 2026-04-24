#include "screens/ui_fuel.h"
#include "screens/ui_fuel_measurement.h"
#include "UI/ui_manager.h"
#include "ui_common.h"

#include <stdint.h>

#define ZH_TITLE "\xE7\x87\x83\xE6\x96\x99\xE5\x88\x97\xE8\xA1\xA8"
#define ZH_NATURAL_GAS "\xE5\xA4\xA9\xE7\x84\xB6\xE6\xB0\x94"
#define ZH_HEAVY_OIL "\xE9\x87\x8D\xE6\xB2\xB9"
#define ZH_LIGHT_OIL "\xE8\xBD\xBB\xE6\xB2\xB9"
#define ZH_PROPANE "\xE4\xB8\x99\xE7\x83\xB7"
#define ZH_BUTANE "\xE4\xB8\x81\xE7\x83\xB7"
#define ZH_WOOD "\xE6\x9C\xA8\xE6\x9D\x90"
#define ZH_COAL "\xE7\x85\xA4"
#define ZH_BIODIESEL "\xE7\x94\x9F\xE7\x89\xA9\xE6\x9F\xB4\xE6\xB2\xB9"
#define ZH_KEROSENE "\xE7\x85\xA4\xE6\xB2\xB9"

typedef struct {
    const char *zh;
    const char *en;
} fuel_item_t;

static const fuel_item_t fuel_list_items[] = {
    {ZH_NATURAL_GAS, "Natural Gas"},
    {ZH_HEAVY_OIL, "Heavy Oil"},
    {ZH_LIGHT_OIL, "Light Oil"},
    {ZH_PROPANE, "Propane"},
    {ZH_BUTANE, "Butane"},
    {ZH_WOOD, "Wood"},
    {ZH_COAL, "Coal"},
    {ZH_BIODIESEL, "Biodiesel"},
    {ZH_KEROSENE, "Kerosene"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void fuel_event(lv_event_t *e)
{
    uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    ui_fuel_measurement_set_item(fuel_list_items[index].zh, fuel_list_items[index].en);
    ui_menu_navigate(UI_MENU_FUEL_MEASUREMENT);
}

lv_obj_t *ui_fuel_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    ui_create_title(screen, ui_lang(ZH_TITLE, "Fuel List"));
    ui_create_nav_bar(screen);
    list = ui_create_page_list(screen);

    for(uint32_t i = 0; i < sizeof(fuel_list_items) / sizeof(fuel_list_items[0]); i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_obj_add_event_cb(btn, fuel_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_label_set_text(label, ui_lang(fuel_list_items[i].zh, fuel_list_items[i].en));
        ui_apply_btn_text_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }

    return screen;
}
