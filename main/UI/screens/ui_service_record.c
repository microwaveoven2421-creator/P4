#include "screens/ui_service_record.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

typedef struct {
    const char *label_zh;
    const char *label_en;
    const char *value;
} service_record_item_t;

static const service_record_item_t service_record_items[] = {
    {"Service counter[h]", "Service counter[h]", "123.1"},
    {"service done", "service done", "----"},
    {"service done", "service done", "----"},
    {"service done", "service done", "----"},
    {"service done", "service done", "----"},
    {"service done", "service done", "----"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t *ui_service_record_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list = NULL;
    int i;

    ui_create_title(screen,
                    ui_lang("\xE7\xBB\xB4\xE6\x8A\xA4\xE8\xAE\xB0\xE5\xBD\x95",
                            "Service Record"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(i = 0; i < (int)(sizeof(service_record_items) / sizeof(service_record_items[0])); i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_t *value = lv_label_create(btn);

        lv_label_set_text(label, ui_lang(service_record_items[i].label_zh, service_record_items[i].label_en));
        ui_apply_btn_text_style(label);
        lv_obj_set_width(label, 220);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        lv_label_set_text(value, service_record_items[i].value);
        ui_apply_btn_text_style(value);
        lv_obj_set_width(value, 160);
        lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    return screen;
}
