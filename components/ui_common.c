#include "ui_common.h"
#include "ui.h"

const char* ui_lang(const char* zh, const char* en)
{
    return (ui_get_language() == LANG_ZH) ? zh : en;
}

lv_obj_t* ui_create_title(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);
    return label;
}

lv_obj_t* ui_create_back_btn(lv_obj_t *parent, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 90, 44);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, ui_lang("返回","Back"));
    lv_obj_center(label);

    return btn;
}