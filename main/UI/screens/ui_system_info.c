#include "screens/ui_system_info.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

static const char *system_info_items[][2] = {
    {"系统版本", "System Version"},
    {"设备型号", "Device Model"},
    {"序列号", "Serial Number"},
    {"保留项 1", "Reserved 1"},
    {"保留项 2", "Reserved 2"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t* ui_system_info_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;

    ui_create_title(screen, ui_lang("系统信息", "System Info"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(int i = 0; i < 5; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_label_set_text(label, ui_lang(system_info_items[i][0], system_info_items[i][1]));
        ui_apply_btn_text_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }

    return screen;
}
