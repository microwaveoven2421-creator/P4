#include "screens/ui_pressure.h"
#include "UI/ui_manager.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>

#define ZH_TITLE "\xE5\x8E\x8B\xE5\x8A\x9B\xE6\xB5\x8B\xE9\x87\x8F"
#define ZH_START "\xE5\xBC\x80\xE5\xA7\x8B"
#define ZH_STOP "\xE5\x81\x9C\xE6\xAD\xA2"
#define ZH_SAVE "\xE4\xBF\x9D\xE5\xAD\x98"
#define ZH_PRINT "\xE6\x89\x93\xE5\x8D\xB0"
#define ZH_PAGE1 "\xE7\xAC\xAC""1""\xE9\xA1\xB5\xE5\x8E\x8B\xE5\x8A\x9B\xE6\xB5\x8B\xE9\x87\x8F\xE9\xA1\xB9\xE5\x8D\xA0\xE4\xBD\x8D"
#define ZH_PAGE2 "\xE7\xAC\xAC""2""\xE9\xA1\xB5\xE5\x8E\x8B\xE5\x8A\x9B\xE6\xB5\x8B\xE9\x87\x8F\xE9\xA1\xB9\xE5\x8D\xA0\xE4\xBD\x8D"
#define ZH_PAGE3 "\xE7\xAC\xAC""3""\xE9\xA1\xB5\xE5\x8E\x8B\xE5\x8A\x9B\xE6\xB5\x8B\xE9\x87\x8F\xE9\xA1\xB9\xE5\x8D\xA0\xE4\xBD\x8D"

typedef struct {
    const char *hint_zh;
    const char *hint_en;
} pressure_page_t;

typedef struct {
    lv_obj_t *start_label;
    bool running;
} pressure_state_t;

static pressure_state_t g_state;

static const pressure_page_t pressure_pages[] = {
    {ZH_PAGE1, "Pressure measurement page 1 placeholder"},
    {ZH_PAGE2, "Pressure measurement page 2 placeholder"},
    {ZH_PAGE3, "Pressure measurement page 3 placeholder"},
};

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void update_start_label(void)
{
    lv_label_set_text(g_state.start_label, ui_lang(g_state.running ? ZH_STOP : ZH_START,
                                                   g_state.running ? "Stop" : "Start"));
}

static void start_stop_event(lv_event_t *e)
{
    (void)e;
    g_state.running = !g_state.running;
    update_start_label();
}

static void empty_event(lv_event_t *e)
{
    (void)e;
}

static lv_obj_t *create_title(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    ui_apply_title_text_style(label);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 26);

    return label;
}

static lv_obj_t *create_indicator(lv_obj_t *parent)
{
    lv_obj_t *indicator = lv_obj_create(parent);

    lv_obj_remove_style_all(indicator);
    lv_obj_set_size(indicator, 90, 16);
    lv_obj_align(indicator, LV_ALIGN_TOP_MID, 0, 78);
    lv_obj_set_layout(indicator, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(indicator, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicator, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(indicator, 8, 0);
    lv_obj_clear_flag(indicator, LV_OBJ_FLAG_SCROLLABLE);

    for(uint32_t i = 0; i < 3; i++) {
        lv_obj_t *dot = lv_obj_create(indicator);

        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, i == 0 ? 28 : 12, 12);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, i == 0 ? lv_color_hex(0x2563EB) : lv_color_hex(0xCBD5E1), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    }

    return indicator;
}

static void update_indicator(lv_obj_t *indicator, uint32_t active_index)
{
    for(uint32_t i = 0; i < lv_obj_get_child_count(indicator); i++) {
        lv_obj_t *dot = lv_obj_get_child(indicator, i);
        bool active = (i == active_index);

        lv_obj_set_size(dot, active ? 28 : 12, 12);
        lv_obj_set_style_bg_color(dot, active ? lv_color_hex(0x2563EB) : lv_color_hex(0xCBD5E1), 0);
    }
}

static void tileview_event(lv_event_t *e)
{
    lv_obj_t *tileview = lv_event_get_target(e);
    lv_obj_t *indicator = lv_event_get_user_data(e);
    lv_obj_t *active_tile = lv_tileview_get_tile_active(tileview);

    for(uint32_t i = 0; i < 3; i++) {
        if(lv_obj_get_child(tileview, i) == active_tile) {
            update_indicator(indicator, i);
            break;
        }
    }
}

static lv_obj_t *create_menu_btn(lv_obj_t *parent, const char *text, lv_color_t pressed_color, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *label = lv_label_create(btn);

    lv_obj_set_size(btn, 110, 110);
    lv_obj_set_style_radius(btn, 24, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, pressed_color, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x93C5FD), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_label_set_text(label, text);
    ui_apply_btn_text_style(label);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);

    return btn;
}

static void create_bottom_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_t *start_btn;

    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, lv_pct(100), 128);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -82);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xEAF0F6), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_pad_all(bar, 9, 0);
    lv_obj_set_style_pad_column(bar, 16, 0);
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    start_btn = create_menu_btn(bar, "", lv_color_hex(0xDBEAFE), start_stop_event);
    g_state.start_label = lv_label_create(start_btn);
    ui_apply_btn_text_style(g_state.start_label);
    lv_obj_center(g_state.start_label);
    update_start_label();

    create_menu_btn(bar, ui_lang(ZH_SAVE, "Save"), lv_color_hex(0xDBEAFE), empty_event);
    create_menu_btn(bar, ui_lang(ZH_PRINT, "Print"), lv_color_hex(0xDBEAFE), empty_event);
}

static void create_page_card(lv_obj_t *parent, const pressure_page_t *page)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_t *box = lv_obj_create(card);
    lv_obj_t *hint = lv_label_create(box);

    lv_obj_set_size(card, 420, 430);
    lv_obj_set_style_radius(card, 28, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xDCE5EF), 0);
    lv_obj_set_style_shadow_width(card, 8, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_pad_all(card, 18, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_size(box, lv_pct(100), lv_pct(100));
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(box, 22, 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0xF8FAFC), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(box, 2, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(0xCBD5E1), 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    lv_label_set_text(hint, ui_lang(page->hint_zh, page->hint_en));
    ui_apply_btn_text_style(hint);
    lv_obj_set_width(hint, 320);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(hint);
}

static void create_tileview(lv_obj_t *parent, lv_obj_t *indicator)
{
    lv_obj_t *tileview = lv_tileview_create(parent);

    lv_obj_set_size(tileview, 480, 470);
    lv_obj_align(tileview, LV_ALIGN_TOP_MID, 0, 96);
    lv_obj_set_style_bg_opa(tileview, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(tileview, 0, 0);
    lv_obj_set_style_pad_all(tileview, 0, 0);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(tileview, tileview_event, LV_EVENT_VALUE_CHANGED, indicator);

    for(uint32_t i = 0; i < 3; i++) {
        lv_obj_t *tile = lv_tileview_add_tile(tileview,
                                              i,
                                              0,
                                              i == 0 ? LV_DIR_RIGHT :
                                              i == 2 ? LV_DIR_LEFT :
                                              (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));

        lv_obj_set_style_bg_opa(tile, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(tile, 0, 0);
        lv_obj_set_style_pad_all(tile, 0, 0);
        create_page_card(tile, &pressure_pages[i]);
    }
}

lv_obj_t *ui_pressure_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *indicator;

    g_state.running = false;

    create_title(screen, ui_lang(ZH_TITLE, "Pressure"));
    ui_create_nav_bar(screen);
    indicator = create_indicator(screen);
    create_tileview(screen, indicator);
    create_bottom_bar(screen);

    return screen;
}
