#include "ui_input.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>

static lv_obj_t *popup;
static lv_obj_t *panel;
static lv_obj_t *title_label;
static lv_obj_t *ta;
static lv_obj_t *kb;
static lv_obj_t *btn_number;
static lv_obj_t *btn_full;
static lv_obj_t *btn_number_label;
static lv_obj_t *btn_full_label;

static ui_input_cb_t input_cb;
static void *input_user_data;

static void apply_mode_btn_style(lv_obj_t *btn, bool active)
{
    lv_obj_set_style_bg_color(btn, active ? lv_color_hex(0xDCE8F6) : lv_color_hex(0xEEF3F8), 0);
    lv_obj_set_style_border_color(btn, active ? lv_color_hex(0x93A9C2) : lv_color_hex(0xD7E0EA), 0);
}

static void set_keyboard_mode(ui_input_mode_t mode)
{
    if(mode == UI_INPUT_MODE_NUMBER) {
        lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    }
    else {
        lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    }

    apply_mode_btn_style(btn_number, mode == UI_INPUT_MODE_NUMBER);
    apply_mode_btn_style(btn_full, mode == UI_INPUT_MODE_FULL);
}

static void update_language_text(const char *title)
{
    lv_label_set_text(title_label, title ? title : ui_lang("输入", "Input"));
    lv_label_set_text(btn_number_label, ui_lang("数字", "Number"));
    lv_label_set_text(btn_full_label, ui_lang("全键盘", "Keyboard"));
}

static void mode_btn_event_cb(lv_event_t *e)
{
    ui_input_mode_t mode = (ui_input_mode_t)(uintptr_t)lv_event_get_user_data(e);
    set_keyboard_mode(mode);
}

static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_READY) {
        const char *text = lv_textarea_get_text(ta);

        if(input_cb) {
            input_cb(text, input_user_data);
        }

        ui_input_hide();
    }
    else if(code == LV_EVENT_CANCEL) {
        ui_input_hide();
    }
}

void ui_input_init(lv_obj_t *parent)
{
    lv_obj_t *host = parent ? parent : lv_layer_top();

    if(popup) {
        return;
    }

    popup = lv_obj_create(host);
    lv_obj_remove_style_all(popup);
    lv_obj_set_size(popup, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_30, 0);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(popup);
    lv_obj_set_size(panel, 460, 560);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 24, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFBFCFE), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(panel, 8, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_pad_all(panel, 16, 0);

    title_label = lv_label_create(panel);
    lv_label_set_text(title_label, ui_lang("输入", "Input"));
    ui_apply_btn_text_style(title_label);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 4);

    btn_number = lv_button_create(panel);
    lv_obj_set_size(btn_number, 120, 44);
    lv_obj_align(btn_number, LV_ALIGN_TOP_LEFT, 0, 44);
    lv_obj_set_style_radius(btn_number, 14, 0);
    lv_obj_set_style_border_width(btn_number, 1, 0);
    lv_obj_set_style_shadow_width(btn_number, 0, 0);
    lv_obj_add_event_cb(btn_number, mode_btn_event_cb, LV_EVENT_CLICKED, (void *)UI_INPUT_MODE_NUMBER);
    btn_number_label = lv_label_create(btn_number);
    lv_label_set_text(btn_number_label, ui_lang("数字", "Number"));
    ui_apply_btn_text_style(btn_number_label);
    lv_obj_center(btn_number_label);

    btn_full = lv_button_create(panel);
    lv_obj_set_size(btn_full, 120, 44);
    lv_obj_align(btn_full, LV_ALIGN_TOP_RIGHT, 0, 44);
    lv_obj_set_style_radius(btn_full, 14, 0);
    lv_obj_set_style_border_width(btn_full, 1, 0);
    lv_obj_set_style_shadow_width(btn_full, 0, 0);
    lv_obj_add_event_cb(btn_full, mode_btn_event_cb, LV_EVENT_CLICKED, (void *)UI_INPUT_MODE_FULL);
    btn_full_label = lv_label_create(btn_full);
    lv_label_set_text(btn_full_label, ui_lang("全键盘", "Keyboard"));
    ui_apply_btn_text_style(btn_full_label);
    lv_obj_center(btn_full_label);

    ta = lv_textarea_create(panel);
    lv_obj_set_size(ta, 428, 72);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 104);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_style_text_font(ta, ui_font_32(), 0);
    lv_obj_set_style_radius(ta, 18, 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_border_color(ta, lv_color_hex(0xD0D7E2), 0);

    kb = lv_keyboard_create(panel);
    lv_obj_set_size(kb, 428, 340);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(kb, 20, 0);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);

    set_keyboard_mode(UI_INPUT_MODE_FULL);
}

void ui_input_show(const char *title,
                   const char *init_text,
                   ui_input_cb_t cb,
                   void *user_data)
{
    ui_input_show_mode(title, init_text, UI_INPUT_MODE_FULL, cb, user_data);
}

void ui_input_show_mode(const char *title,
                        const char *init_text,
                        ui_input_mode_t mode,
                        ui_input_cb_t cb,
                        void *user_data)
{
    if(!popup) {
        ui_input_init(lv_layer_top());
    }

    input_cb = cb;
    input_user_data = user_data;

    update_language_text(title);
    lv_textarea_set_text(ta, init_text ? init_text : "");
    set_keyboard_mode(mode);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(popup);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
}

void ui_input_hide(void)
{
    if(!popup) {
        return;
    }

    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
}
