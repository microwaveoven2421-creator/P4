#include "screens/ui_home.h"
#include "UI/ui.h"
#include "ui_common.h"
#include "UI/ui_manager.h"
#include "lvgl.h"

#include <stdint.h>

LV_FONT_DECLARE(my_Font_32);

// ================= 数据 =================
typedef struct {
    const char *btn;
    const char *zh;
    const char *en;
} home_item_t;

static const home_item_t home_items[] = {
    {"1", "烟气测量", "Gas"},
    {"2", "压力测量", "Pressure"},
    {"3", "温度测量", "Temp"},
    {"4", "设置", "Settings"},
    {"5", "校准", "Calib"},
    {"6", "...", "..."},
};


// ================= 事件 =================
static void btn_event(lv_event_t *e)
{
    uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    switch(index)
    {
        case 0: //烟气
            ui_menu_navigate(UI_MENU_FUEL);
            break;

        case 3: //设置
            ui_menu_navigate(UI_MENU_SETTING);
            break;

        default:
            break;
    }
}


// ================= LOGO =================
static void create_logo(lv_obj_t *parent)
{
    lv_obj_t *logo_area = lv_obj_create(parent);
    lv_obj_remove_style_all(logo_area);
    lv_obj_set_size(logo_area, 480, 260);
    lv_obj_align(logo_area, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *logo = lv_label_create(logo_area);
    lv_label_set_text(logo, "LOGO");
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *sub = lv_label_create(logo_area);
    lv_label_set_text(sub, ui_lang("主页","Home"));
    lv_obj_set_style_text_font(sub, &my_Font_32, 0);
    lv_obj_align_to(sub, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
}


// ================= 按钮 =================
static lv_obj_t* create_btn(lv_obj_t *parent,
                            const home_item_t *item,
                            uint32_t index)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 140, 160);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    // 按钮
    lv_obj_t *btn = lv_button_create(cont);
    lv_obj_set_size(btn, 110, 110);
    lv_obj_set_style_radius(btn, 20, 0);

    lv_obj_add_event_cb(btn, btn_event,
        LV_EVENT_CLICKED,
        (void*)(uintptr_t)index);

    // 按钮文字
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, item->btn);
    lv_obj_center(label);

    // 说明
    lv_obj_t *cap = lv_label_create(cont);
    lv_label_set_text(cap,
        ui_lang(item->zh, item->en));
    lv_obj_set_width(cap, 140);
    lv_obj_set_style_text_align(cap,
        LV_TEXT_ALIGN_CENTER, 0);

    return cont;
}


// ================= 网格 =================
static void create_grid(lv_obj_t *parent)
{
    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 460, 400);
    lv_obj_align(grid, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    static int32_t col[] = {140,140,140, LV_GRID_TEMPLATE_LAST};
    static int32_t row[] = {180,180, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(grid, col, row);

    for(int i = 0; i < 6; i++)
    {
        lv_obj_t *item =
            create_btn(grid, &home_items[i], i);

        lv_obj_set_grid_cell(item,
            LV_GRID_ALIGN_CENTER, i%3,1,
            LV_GRID_ALIGN_CENTER, i/3,1);
    }
}


// ================= 创建页面 =================
lv_obj_t* ui_home_create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(screen,
        lv_color_hex(0xFFFFFF), 0);

    create_logo(screen);
    create_grid(screen);

    return screen;
}
