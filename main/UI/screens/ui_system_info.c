#include "screens/ui_system_info.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

#include <stdint.h>

typedef enum {
    DEVICE_INFO_FW_VERSION = 0,
    DEVICE_INFO_SERIAL_NUMBER,
    DEVICE_INFO_SERVICE_RECORD,
    DEVICE_INFO_MANUFACTURER_INFO,
    DEVICE_INFO_COUNT
} device_info_id_t;

typedef struct {
    const char *label_zh;
    const char *label_en;
    const char *value;
} device_info_item_t;

static const device_info_item_t device_info_items[DEVICE_INFO_COUNT] = {
    {"\xE5\x9B\xBA\xE4\xBB\xB6\xE7\x89\x88\xE6\x9C\xAC", "Firmware Version", "V1.0.0"},
    {"\xE8\xAE\xBE\xE5\xA4\x87\xE5\xBA\x8F\xE5\x88\x97\xE5\x8F\xB7", "Device Serial Number", "SN00000001"},
    {"\xE7\xBB\xB4\xE6\x8A\xA4\xE8\xAE\xB0\xE5\xBD\x95", "Service Record", ""},
    {"\xE5\x8E\x82\xE5\xAE\xB6\xE4\xBF\xA1\xE6\x81\xAF", "Manufacturer Info", ""},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void item_event(lv_event_t *e)
{
    device_info_id_t item = (device_info_id_t)(uintptr_t)lv_event_get_user_data(e);

    switch(item) {
        case DEVICE_INFO_SERVICE_RECORD:
            ui_menu_navigate(UI_MENU_SERVICE_RECORD);
            break;
        case DEVICE_INFO_MANUFACTURER_INFO:
            ui_menu_navigate(UI_MENU_MANUFACTURER_INFO);
            break;
        default:
            break;
    }
}

lv_obj_t* ui_system_info_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;
    int i;

    ui_create_title(screen,
                    ui_lang("\xE8\xAE\xBE\xE5\xA4\x87\xE4\xBF\xA1\xE6\x81\xAF",
                            "Device Info"));
    ui_create_nav_bar(screen);
    list = ui_create_page_list(screen);

    for(i = 0; i < DEVICE_INFO_COUNT; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_t *value = lv_label_create(btn);

        lv_obj_add_event_cb(btn, item_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_label_set_text(label, ui_lang(device_info_items[i].label_zh, device_info_items[i].label_en));
        ui_apply_btn_text_style(label);
        lv_obj_set_width(label, 220);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        lv_label_set_text(value, device_info_items[i].value);
        ui_apply_btn_text_style(value);
        lv_obj_set_width(value, 170);
        lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    return screen;
}
