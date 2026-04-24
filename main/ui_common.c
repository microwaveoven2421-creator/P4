#include "ui_common.h"
#include "ui.h"
#include "UI/ui_manager.h"
#include "comp/ui_input.h"

#include <stdio.h>

LV_FONT_DECLARE(my_Font_32);

static char g_nav_keyboard_value[64] = "";

static void ui_style_text(lv_obj_t *obj, lv_color_t color)
{
    lv_obj_set_style_text_font(obj, &my_Font_32, 0);
    lv_obj_set_style_text_color(obj, color, 0);
}

const char* ui_lang(const char* zh, const char* en)
{
    return (ui_get_language() == LANG_ZH) ? zh : en;
}

const lv_font_t* ui_font_32(void)
{
    return &my_Font_32;
}

lv_obj_t* ui_create_screen(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF5F7FA), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    return screen;
}

lv_obj_t* ui_create_title(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    ui_style_text(label, lv_color_hex(0x1F2937));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 24);

    return label;
}

lv_obj_t* ui_create_back_btn(lv_obj_t *parent, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *label;

    lv_obj_set_size(btn, 112, 44);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 16, 18);
    lv_obj_set_style_radius(btn, 14, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEEF3F8), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD3DCE6), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_pad_hor(btn, 12, 0);
    lv_obj_set_style_pad_ver(btn, 6, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDDE7F2), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x8DA2B8), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(btn);
    lv_label_set_text(label, ui_lang("返回", "Back"));
    ui_style_text(label, lv_color_hex(0x334155));
    lv_obj_center(label);

    return btn;
}

static void nav_input_cb(const char *text, void *user_data)
{
    (void)user_data;
    snprintf(g_nav_keyboard_value, sizeof(g_nav_keyboard_value), "%s", text ? text : "");
}

static void nav_back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void nav_home_event(lv_event_t *e)
{
    (void)e;
    ui_menu_home();
}

static void nav_keyboard_event(lv_event_t *e)
{
    (void)e;
    ui_input_show_mode(ui_lang("键盘", "Keyboard"),
                       g_nav_keyboard_value,
                       UI_INPUT_MODE_FULL,
                       nav_input_cb,
                       NULL);
}

static lv_obj_t *create_nav_btn(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *label = lv_label_create(btn);

    lv_obj_set_size(btn, 124, 52);
    lv_obj_set_style_radius(btn, 18, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCE8F6), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x93A9C2), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_label_set_text(label, text);
    ui_apply_btn_text_style(label);
    lv_obj_center(label);

    return btn;
}

lv_obj_t* ui_create_nav_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);

    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, lv_pct(100), 82);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xEAF0F6), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_pad_left(bar, 16, 0);
    lv_obj_set_style_pad_right(bar, 16, 0);
    lv_obj_set_style_pad_top(bar, 10, 0);
    lv_obj_set_style_pad_bottom(bar, 10, 0);
    lv_obj_set_style_pad_column(bar, 12, 0);
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    create_nav_btn(bar, ui_lang("返回", "Back"), nav_back_event);
    create_nav_btn(bar, ui_lang("主页", "Home"), nav_home_event);
    create_nav_btn(bar, ui_lang("键盘", "Keyboard"), nav_keyboard_event);

    return bar;
}

lv_obj_t* ui_create_page_list(lv_obj_t *parent)
{
    lv_obj_t *list = lv_obj_create(parent);

    lv_obj_set_size(list, 456, 620);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_radius(list, 28, 0);
    lv_obj_set_style_bg_color(list, lv_color_hex(0xFBFCFE), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 1, 0);
    lv_obj_set_style_border_color(list, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_style_shadow_width(list, 8, 0);
    lv_obj_set_style_shadow_opa(list, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(list, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_pad_top(list, 16, 0);
    lv_obj_set_style_pad_bottom(list, 18, 0);
    lv_obj_set_style_pad_left(list, 16, 0);
    lv_obj_set_style_pad_right(list, 16, 0);
    lv_obj_set_style_pad_row(list, 10, 0);
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

    return list;
}

lv_obj_t* ui_create_list_btn(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_button_create(parent);

    lv_obj_set_width(btn, lv_pct(100));
    lv_obj_set_height(btn, 76);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEEF3F8), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_pad_left(btn, 22, 0);
    lv_obj_set_style_pad_right(btn, 22, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCE8F6), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x93A9C2), LV_STATE_PRESSED);

    return btn;
}

void ui_apply_btn_text_style(lv_obj_t *label)
{
    ui_style_text(label, lv_color_hex(0x334155));
}

void ui_apply_title_text_style(lv_obj_t *label)
{
    ui_style_text(label, lv_color_hex(0x0F172A));
}
