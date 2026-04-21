#include "screens/ui_fuel.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

static const char *fuel_list_items[][2] = {
    {"Natural Gas", "Natural Gas"},
    {"Heavy Oil", "Heavy Oil"},
    {"Light Oil", "Light Oil"},
    {"Propane", "Propane"},
    {"Butane", "Butane"},
    {"Wood", "Wood"},
    {"Coal", "Coal"},
    {"Biodiesel", "Biodiesel"},
    {"Kerosene", "Kerosene"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t* ui_fuel_create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);

    ui_create_title(screen, ui_lang("Fuel List", "Fuel List"));
    ui_create_back_btn(screen, back_event);

    lv_obj_t *list = lv_obj_create(screen);
    lv_obj_set_size(list, 440, 650);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < 9; i++) {
        lv_obj_t *btn = lv_button_create(list);
        lv_obj_set_size(btn, 440, 60);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, ui_lang(fuel_list_items[i][0], fuel_list_items[i][1]));
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, 0);
    }

    return screen;
}
