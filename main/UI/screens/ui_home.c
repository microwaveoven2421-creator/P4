#include "screens/ui_home.h"
#include "UI/ui.h"
#include "UI/ui_manager.h"
#include "ui_common.h"

#include <stdint.h>

#define ZH_HOME "\xE4\xB8\xBB\xE9\xA1\xB5"
#define ZH_GAS "\xE7\x83\x9F\xE6\xB0\x94\xE6\xB5\x8B\xE9\x87\x8F"
#define ZH_PRESSURE "\xE5\x8E\x8B\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F"
#define ZH_TEMPERATURE "\xE6\xB8\xA9\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F"
#define ZH_SETTING "\xE8\xAE\xBE\xE7\xBD\xAE"
#define ZH_CALIBRATION "\xE6\xA0\xA1\xE5\x87\x86"
#define ZH_SAVED_FILES "\xE5\xB7\xB2\xE4\xBF\x9D\xE5\xAD\x98\xE6\x96\x87\xE4\xBB\xB6"

typedef struct {
    const char *btn;
    const char *zh;
    const char *en;
} home_item_t;

static const home_item_t home_items[] = {
    {"1", ZH_GAS, "Gas"},
    {"2", ZH_PRESSURE, "Differential Pressure"},
    {"3", ZH_TEMPERATURE, "Temperature Difference"},
    {"4", ZH_SETTING, "Settings"},
    {"5", ZH_CALIBRATION, "Calibration"},
    {"6", ZH_SAVED_FILES, "Saved Files"},
};

static void btn_event(lv_event_t *e)
{
    uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    switch(index) {
        case 0:
            ui_menu_navigate(UI_MENU_FUEL);
            break;
        case 1:
            ui_menu_navigate(UI_MENU_PRESSURE);
            break;
        case 2:
            ui_menu_navigate(UI_MENU_TEMPERATURE);
            break;
        case 3:
            ui_menu_navigate(UI_MENU_SETTING);
            break;
        case 4:
            ui_menu_navigate(UI_MENU_CALIBRATION);
            break;
        case 5:
            ui_menu_navigate(UI_MENU_SAVED_FILES);
            break;
        default:
            break;
    }
}

static void create_logo(lv_obj_t *parent)
{
    lv_obj_t *logo_area = lv_obj_create(parent);
    lv_obj_t *logo = lv_label_create(logo_area);
    lv_obj_t *sub = lv_label_create(logo_area);

    lv_obj_remove_style_all(logo_area);
    lv_obj_set_size(logo_area, lv_pct(100), 240);
    lv_obj_align(logo_area, LV_ALIGN_TOP_MID, 0, 28);

    lv_label_set_text(logo, "LOGO");
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(logo, lv_color_hex(0x475569), 0);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, -18);

    lv_label_set_text(sub, ui_lang(ZH_HOME, "Home"));
    ui_apply_btn_text_style(sub);
    lv_obj_align_to(sub, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
}

static lv_obj_t *create_btn(lv_obj_t *parent, const home_item_t *item, uint32_t index)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_t *btn = lv_button_create(cont);
    lv_obj_t *label = lv_label_create(btn);
    lv_obj_t *cap = lv_label_create(cont);

    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 150, 150);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_size(btn, 130, 100);
    lv_obj_set_style_radius(btn, 18, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD0D7E2), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xE5ECF4), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xB8C5D3), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, btn_event, LV_EVENT_CLICKED, (void *)(uintptr_t)index);

    lv_label_set_text(label, item->btn);
    lv_obj_set_style_text_font(label, ui_font_32(), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x334155), 0);
    lv_obj_center(label);

    lv_label_set_text(cap, ui_lang(item->zh, item->en));
    lv_obj_set_style_pad_top(cap, 10, 0);
    ui_apply_btn_text_style(cap);
    lv_obj_set_width(cap, 144);
    lv_label_set_long_mode(cap, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(cap, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(cap, ui_font_32(), 0);

    return cont;
}

static void create_grid(lv_obj_t *parent)
{
    static int32_t col[] = {160, 160, 160, LV_GRID_TEMPLATE_LAST};
    static int32_t row[] = {170, 170, LV_GRID_TEMPLATE_LAST};
    lv_obj_t *grid = lv_obj_create(parent);

    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 480, 360);
    lv_obj_align(grid, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, col, row);

    for(int i = 0; i < 6; i++) {
        lv_obj_t *item = create_btn(grid, &home_items[i], i);
        lv_obj_set_grid_cell(item, LV_GRID_ALIGN_CENTER, i % 3, 1, LV_GRID_ALIGN_CENTER, i / 3, 1);
    }
}

lv_obj_t *ui_home_create(void)
{
    lv_obj_t *screen = ui_create_screen();

    create_logo(screen);
    create_grid(screen);

    return screen;
}
