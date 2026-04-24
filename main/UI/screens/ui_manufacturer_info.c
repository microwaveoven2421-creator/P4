#include "screens/ui_manufacturer_info.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

typedef struct {
    const char *label_zh;
    const char *label_en;
    const char *value;
} manufacturer_info_item_t;

static const manufacturer_info_item_t manufacturer_info_items[] = {
    {"\xE5\x8E\x82\xE5\xAE\xB6\xE7\x94\xB5\xE8\xAF\x9D", "Phone", "----"},
    {"\xE5\x9C\xB0\xE5\x9D\x80", "Address", "----"},
    {"\xE9\x82\xAE\xE7\xAE\xB1", "Email", "----"},
    {"\xE9\xA2\x84\xE7\x95\x99""1", "Reserved 1", "----"},
    {"\xE9\xA2\x84\xE7\x95\x99""2", "Reserved 2", "----"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

lv_obj_t *ui_manufacturer_info_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list = NULL;
    int i;

    ui_create_title(screen,
                    ui_lang("\xE5\x8E\x82\xE5\xAE\xB6\xE4\xBF\xA1\xE6\x81\xAF",
                            "Manufacturer Info"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(i = 0; i < (int)(sizeof(manufacturer_info_items) / sizeof(manufacturer_info_items[0])); i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_t *value = lv_label_create(btn);

        lv_label_set_text(label, ui_lang(manufacturer_info_items[i].label_zh, manufacturer_info_items[i].label_en));
        ui_apply_btn_text_style(label);
        lv_obj_set_width(label, 180);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        lv_label_set_text(value, manufacturer_info_items[i].value);
        ui_apply_btn_text_style(value);
        lv_obj_set_width(value, 200);
        lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    return screen;
}
