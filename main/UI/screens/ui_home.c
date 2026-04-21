#include "ui_home.h"
#include "ui.h"
#include "ui_common.h"

// 主页界面实现
static void btn_event(lv_event_t *e)
{
    uint32_t id = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    if (id == 0)
        ui_show_fuel();
    if (id == 1)
        ui_show_pressure();
    if (id == 2)
        ui_show_temperature();
    if (id == 3)
        ui_show_setting();
    if (id == 4)
        ui_show_calib();
    if (id == 5)
        ui_show_about();
}

// 创建主页界面
lv_obj_t* ui_home_create(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);

    ui_create_title(screen, ui_lang("主页","Home"));

    // 示例按钮
    lv_obj_t *btn = lv_button_create(screen);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, btn_event, LV_EVENT_CLICKED, (void*)0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "1");
    lv_obj_center(label);

    return screen;
}