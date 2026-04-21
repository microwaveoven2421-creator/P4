#include "screens/ui_fuel.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

static const char *fuel_list_items[][2] = {
    {"天然气", "Natural Gas"},
    {"重油", "Heavy Oil"},
    {"轻油", "Light Oil"},
    {"丙烷", "Propane"},
    {"丁烷", "Butane"},
    {"木材", "Wood"},
    {"煤", "Coal"},
    {"生物柴油", "Biodiesel"},
    {"煤油", "Kerosene"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t* ui_fuel_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    ui_create_title(screen, ui_lang("燃料列表", "Fuel List"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(int i = 0; i < 9; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_label_set_text(label, ui_lang(fuel_list_items[i][0], fuel_list_items[i][1]));
        ui_apply_btn_text_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }

    return screen;
}
