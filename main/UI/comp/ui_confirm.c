#include "ui_confirm.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>

#define ZH_RESTORE_DEFAULT_CONFIRM "\xE7\xA1\xAE\xE5\xAE\x9A\xE8\xA6\x81\xE6\x81\xA2\xE5\xA4\x8D\xE9\xBB\x98\xE8\xAE\xA4\xE8\xAE\xBE\xE7\xBD\xAE\xE5\x90\x97?"
#define ZH_RESTORE_DEFAULT_DONE "\xE5\xB7\xB2\xE6\x81\xA2\xE5\xA4\x8D\xE9\xBB\x98\xE8\xAE\xA4\xE8\xAE\xBE\xE7\xBD\xAE"
#define ZH_CANCEL "\xE5\x8F\x96\xE6\xB6\x88"
#define ZH_OK "\xE7\xA1\xAE\xE5\xAE\x9A"

static lv_obj_t *popup;
static lv_obj_t *panel;
static lv_obj_t *label;
static lv_obj_t *cancel_label_obj;
static lv_obj_t *ok_label_obj;
static lv_obj_t *ok_btn_obj;

static ui_confirm_cb_t confirm_cb;
static void *confirm_user_data;
static lv_obj_t *cancel_btn_obj;

static bool popup_ready(void)
{
    return popup && lv_obj_is_valid(popup) && label && lv_obj_is_valid(label);
}

static void hide_popup(void)
{
    if(popup_ready()) {
        lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
    }
}

static void update_popup_language(void)
{
    if(cancel_label_obj && lv_obj_is_valid(cancel_label_obj)) {
        lv_label_set_text(cancel_label_obj, ui_lang(ZH_CANCEL, "Cancel"));
    }

    if(ok_label_obj && lv_obj_is_valid(ok_label_obj)) {
        lv_label_set_text(ok_label_obj, ui_lang(ZH_OK, "OK"));
    }
}

static void btn_event(lv_event_t *e)
{
    bool is_ok = (bool)(uintptr_t)lv_event_get_user_data(e);
    ui_confirm_cb_t cb = confirm_cb;
    void *user_data = confirm_user_data;

    hide_popup();

    if(is_ok && cb) {
        cb(user_data);
    }
}

void ui_confirm_init(lv_obj_t *parent)
{
    lv_obj_t *host = parent;

    (void)parent;

    if(popup_ready()) {
        lv_obj_move_foreground(popup);
        return;
    }

    host = lv_layer_top();

    popup = lv_obj_create(host);
    lv_obj_remove_style_all(popup);
    lv_obj_set_size(popup, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(popup, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_30, 0);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);

    panel = lv_obj_create(popup);
    lv_obj_set_size(panel, 360, 200);
    lv_obj_center(panel);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    label = lv_label_create(panel);
    lv_label_set_text(label, ui_lang(ZH_RESTORE_DEFAULT_CONFIRM, "Restore default settings?"));
    lv_obj_set_width(label, 300);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    ui_apply_btn_text_style(label);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 18);

    {
        lv_obj_t *btn_cancel = lv_button_create(panel);
        cancel_btn_obj = btn_cancel;
        cancel_label_obj = lv_label_create(btn_cancel);

        lv_obj_set_size(btn_cancel, 120, 50);
        lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 20, -10);
        lv_obj_add_event_cb(btn_cancel, btn_event, LV_EVENT_CLICKED, (void *)0);

        lv_label_set_text(cancel_label_obj, ui_lang(ZH_CANCEL, "Cancel"));
        ui_apply_btn_text_style(cancel_label_obj);
        lv_obj_center(cancel_label_obj);
    }

    {
        lv_obj_t *btn_ok = lv_button_create(panel);
        ok_btn_obj = btn_ok;
        ok_label_obj = lv_label_create(btn_ok);

        lv_obj_set_size(btn_ok, 120, 50);
        lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
        lv_obj_add_event_cb(btn_ok, btn_event, LV_EVENT_CLICKED, (void *)1);

        lv_label_set_text(ok_label_obj, ui_lang(ZH_OK, "OK"));
        ui_apply_btn_text_style(ok_label_obj);
        lv_obj_center(ok_label_obj);
    }
}

void ui_confirm_show(const char *text, ui_confirm_cb_t ok_cb, void *user_data)
{
    if(!popup_ready()) {
        ui_confirm_init(lv_layer_top());
    }

    if(!popup_ready()) {
        return;
    }

    update_popup_language();
    if(cancel_btn_obj && lv_obj_is_valid(cancel_btn_obj)) {
        lv_obj_clear_flag(cancel_btn_obj, LV_OBJ_FLAG_HIDDEN);
    }
    if(ok_btn_obj && lv_obj_is_valid(ok_btn_obj)) {
        lv_obj_align(ok_btn_obj, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    }
    lv_label_set_text(label, text ? text : ui_lang(ZH_RESTORE_DEFAULT_CONFIRM, "Restore default settings?"));
    confirm_cb = ok_cb;
    confirm_user_data = user_data;

    lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(popup);
}

void ui_confirm_show_ok(const char *text, ui_confirm_cb_t ok_cb, void *user_data)
{
    if(!popup_ready()) {
        ui_confirm_init(lv_layer_top());
    }

    if(!popup_ready()) {
        return;
    }

    update_popup_language();
    if(cancel_btn_obj && lv_obj_is_valid(cancel_btn_obj)) {
        lv_obj_add_flag(cancel_btn_obj, LV_OBJ_FLAG_HIDDEN);
    }
    if(ok_btn_obj && lv_obj_is_valid(ok_btn_obj)) {
        lv_obj_align(ok_btn_obj, LV_ALIGN_BOTTOM_MID, 0, -10);
    }
    lv_label_set_text(label, text ? text : "");
    confirm_cb = ok_cb;
    confirm_user_data = user_data;

    lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(popup);
}

void ui_confirm_hide(void)
{
    hide_popup();
}
