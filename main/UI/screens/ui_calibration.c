#include "ui_common.h"
#include "ui_manager.h"

static const char *items[][2] = {
    {"系数调整", "Coefficient"},
    {"标气测试", "Gas Test"},
    {"压力校准", "Pressure"},
    {"温度校准", "Temperature"},
};

static void back_event(lv_event_t *e)
{
    ui_menu_back();
}

lv_obj_t* ui_calibration_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    ui_create_title(screen, ui_lang("校准", "Calibration"));
    ui_create_back_btn(screen, back_event);

    list = ui_create_page_list(screen);

    for(int i = 0; i < 4; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_label_set_text(label, ui_lang(items[i][0], items[i][1]));
        ui_apply_btn_text_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }

    return screen;
}