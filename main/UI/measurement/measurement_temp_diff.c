/**
 * @file ui_Temp_Diff_Menu.c
 * @brief 差温测量界面
 *
 * 2页：图表页(page1) + 详细数据页(page2)，手势左右滑动切换。
 *
 * 优化策略（与 Pressure 对齐）:
 *   - 图表缩小至与压力界面相同尺寸，减少渲染开销
 *   - 绝对定位，无 flex 布局
 *   - 渐变线用绘制回调，不创建额外对象
 *   - 所有子对象创建时直接设 EVENT_BUBBLE
 *   - 双系列数据用 set_all_value 批量初始化
 *   - page2 取消顶部空白，从 y=0 开始排列
 */

#include "measurement_ui.h"

#define MENU_WIDTH          480
#define MENU_HEIGHT         620
#define GRAD_HEIGHT         5

#define COLOR_BG_NORMAL     0xD3D3D3
#define COLOR_TEXT_NORMAL   0x000000
#define COLOR_RED           0xFF0000
#define COLOR_BLUE          0x0000FF
#define COLOR_GREEN         0x008000

/* 图表高度与 Pressure 对齐 */
#define ROW1_HEIGHT         120
#define ROW_NORMAL_HEIGHT   60
#define CHART_H             ((ROW1_HEIGHT + GRAD_HEIGHT) + (ROW_NORMAL_HEIGHT + GRAD_HEIGHT) * 4 - GRAD_HEIGHT)

/* page2 功能块高度：3块平分700px */
#define PAGE2_BLOCK_HEIGHT  ((MENU_HEIGHT - GRAD_HEIGHT * 3) / 3)

#define TOTAL_PAGES         2
#define SWIPE_THRESHOLD     50

static lv_point_t press_point;
static bool is_pressing = false;

/* ------------------------------------------------------------------ */
/* 渐变线绘制回调                                                       */
/* ------------------------------------------------------------------ */
static void row_draw_cb(lv_event_t * e) {
    if (lv_event_get_code(e) != LV_EVENT_DRAW_POST) return;

    lv_obj_t * obj = lv_event_get_target(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(obj, &a);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_opa = LV_OPA_COVER;
    dsc.bg_color = lv_color_hex(0xFFFFFF);
    dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
    dsc.bg_grad.stops[0].color = lv_color_hex(0xFFFFFF);
    dsc.bg_grad.stops[0].opa   = LV_OPA_COVER;
    dsc.bg_grad.stops[0].frac  = 88;
    dsc.bg_grad.stops[1].color = lv_color_hex(0x000000);
    dsc.bg_grad.stops[1].opa   = LV_OPA_COVER;
    dsc.bg_grad.stops[1].frac  = 255;
    dsc.bg_grad.stops_count    = 2;

    lv_area_t grad_area = {a.x1, a.y2 - GRAD_HEIGHT + 1, a.x2, a.y2};
    lv_draw_rect(layer, &dsc, &grad_area);
}

/* ------------------------------------------------------------------ */
/* 工具函数                                                             */
/* ------------------------------------------------------------------ */
static lv_obj_t * create_row(lv_obj_t * parent, int16_t height, int16_t y_pos) {
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, MENU_WIDTH, height + GRAD_HEIGHT);
    lv_obj_set_pos(item, 0, y_pos);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_set_style_bg_color(item, lv_color_hex(COLOR_BG_NORMAL), 0);
    lv_obj_set_style_text_color(item, lv_color_hex(COLOR_TEXT_NORMAL), 0);
    lv_obj_add_event_cb(item, row_draw_cb, LV_EVENT_DRAW_POST, NULL);
    lv_obj_add_flag(item, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_CLICKABLE);
    return item;
}

static lv_obj_t * create_label_bubble(lv_obj_t * parent, const char * text,
                                       const lv_font_t * font, uint32_t color) {
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    if (font)  lv_obj_set_style_text_font(label, font, 0);
    if (color) lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);
    return label;
}

/* ------------------------------------------------------------------ */
/* 温度计图标（page2 用）                                               */
/* ------------------------------------------------------------------ */
static lv_obj_t * create_thermometer_icon(lv_obj_t * parent, uint32_t color) {
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 40, 100);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t * tube = lv_obj_create(cont);
    lv_obj_remove_style_all(tube);
    lv_obj_set_size(tube, 16, 60);
    lv_obj_align(tube, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(tube, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tube, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(tube, 3, 0);
    lv_obj_set_style_border_color(tube, lv_color_hex(color), 0);
    lv_obj_set_style_radius(tube, 8, 0);
    lv_obj_add_flag(tube, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(tube, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * bulb = lv_obj_create(cont);
    lv_obj_remove_style_all(bulb);
    lv_obj_set_size(bulb, 32, 32);
    lv_obj_align(bulb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(bulb, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(bulb, lv_color_hex(color), 0);
    lv_obj_set_style_radius(bulb, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_flag(bulb, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(bulb, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * liquid = lv_obj_create(cont);
    lv_obj_remove_style_all(liquid);
    lv_obj_set_size(liquid, 8, 45);
    lv_obj_align(liquid, LV_ALIGN_TOP_MID, 0, 18);
    lv_obj_set_style_bg_opa(liquid, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(liquid, lv_color_hex(color), 0);
    lv_obj_add_flag(liquid, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(liquid, LV_OBJ_FLAG_CLICKABLE);

    return cont;
}

/* ------------------------------------------------------------------ */
/* 页面切换                                                             */
/* ------------------------------------------------------------------ */
static void show_page(ui_temp_diff_menu_t * menu, uint8_t page_index) {
    if (menu == NULL) return;
    if (menu->page1) lv_obj_add_flag(menu->page1, LV_OBJ_FLAG_HIDDEN);
    if (menu->page2) lv_obj_add_flag(menu->page2, LV_OBJ_FLAG_HIDDEN);
    switch (page_index) {
        case 0:  if (menu->page1) lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_HIDDEN); break;
        case 1:  if (menu->page2) lv_obj_clear_flag(menu->page2, LV_OBJ_FLAG_HIDDEN); break;
        default: if (menu->page1) lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_HIDDEN); break;
    }
    menu->current_page = page_index;
}

static void touch_event_cb(lv_event_t * e) {
    ui_temp_diff_menu_t * menu = (ui_temp_diff_menu_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (menu == NULL) return;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &press_point);
            is_pressing = true;
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!is_pressing) return;
        is_pressing = false;
        lv_indev_t * indev = lv_indev_get_act();
        if (indev == NULL) return;
        lv_point_t rp;
        lv_indev_get_point(indev, &rp);
        int32_t dx = rp.x - press_point.x;
        if      (dx < -SWIPE_THRESHOLD) show_page(menu, (menu->current_page + 1) % TOTAL_PAGES);
        else if (dx >  SWIPE_THRESHOLD) show_page(menu, (menu->current_page + TOTAL_PAGES - 1) % TOTAL_PAGES);
    }
}

/* ------------------------------------------------------------------ */
/* Page1 - 图表页                                                     */
/*                                                                      */
/* 行高分配（共700px）：                                                 */
/*   chart_row : CHART_H + GRAD_HEIGHT  ≈ 425+5                        */
/*   t2_row    : ROW_NORMAL_HEIGHT(60) + GRAD_HEIGHT                   */
/*   t1_row    : ROW_NORMAL_HEIGHT(60) + GRAD_HEIGHT                   */
/*   diff_row  : 剩余高度                                               */
/* ------------------------------------------------------------------ */
static void create_page1(ui_temp_diff_menu_t * menu) {
    menu->page1 = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->page1);
    lv_obj_set_size(menu->page1, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_set_pos(menu->page1, 0, 0);
    lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(menu->page1, LV_OBJ_FLAG_EVENT_BUBBLE);

    int16_t y = 0;

    /* 图表区域 */
    lv_obj_t * chart_row = create_row(menu->page1, CHART_H, y);

    menu->chart = lv_chart_create(chart_row);
    lv_obj_set_size(menu->chart, MENU_WIDTH - 80, CHART_H - 40);
    lv_obj_center(menu->chart);
    lv_obj_add_flag(menu->chart, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(menu->chart, LV_OBJ_FLAG_CLICKABLE);

    lv_chart_set_type(menu->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(menu->chart, 30);
    /* 只设置主 Y 轴范围，不再设置次轴 */
    lv_chart_set_range(menu->chart, LV_CHART_AXIS_PRIMARY_Y, 18, 22);
    lv_chart_set_div_line_count(menu->chart, 3, 5);

    lv_obj_set_style_bg_color(menu->chart, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(menu->chart, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(menu->chart, 1, 0);
    lv_obj_set_style_line_color(menu->chart, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_line_width(menu->chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(menu->chart, 0, 0, LV_PART_INDICATOR);

    /* 两个系列都绑定到主 Y 轴 */
    menu->chart_series_t2 = lv_chart_add_series(menu->chart, lv_color_hex(COLOR_RED),  LV_CHART_AXIS_PRIMARY_Y);
    menu->chart_series_t1 = lv_chart_add_series(menu->chart, lv_color_hex(COLOR_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    /* 批量初始化数据，LVGL 9.4 内部只标记脏区，不会多次重绘，无需手动关刷新 */
    lv_chart_set_all_value(menu->chart, menu->chart_series_t1, 20);
    lv_chart_set_all_value(menu->chart, menu->chart_series_t2, 20);

    /* 左侧刻度（蓝/T1） */
    lv_obj_t * sl;
    sl = create_label_bubble(chart_row, "22", &lv_font_montserrat_16, COLOR_BLUE);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_TOP,    -5, 0);
    sl = create_label_bubble(chart_row, "20", &lv_font_montserrat_16, COLOR_BLUE);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_MID,    -5, 0);
    sl = create_label_bubble(chart_row, "18", &lv_font_montserrat_16, COLOR_BLUE);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 0);

    /* 右侧刻度（红/T2）— 仅作视觉提示，不参与轴逻辑 */
    sl = create_label_bubble(chart_row, "22", &lv_font_montserrat_16, COLOR_RED);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_RIGHT_TOP,    5, 0);
    sl = create_label_bubble(chart_row, "20", &lv_font_montserrat_16, COLOR_RED);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_RIGHT_MID,    5, 0);
    sl = create_label_bubble(chart_row, "18", &lv_font_montserrat_16, COLOR_RED);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);

    y += CHART_H + GRAD_HEIGHT;

    /* T2行 - 红色 */
    lv_obj_t * t2_row = create_row(menu->page1, ROW_NORMAL_HEIGHT, y);
    menu->t2_left_label  = create_label_bubble(t2_row, "(T2) 入口 °C", font_normal_32, COLOR_RED);
    lv_obj_align(menu->t2_left_label,  LV_ALIGN_LEFT_MID,  20,  0);
    menu->t2_right_label = create_label_bubble(t2_row, "--.-", font_normal_32, COLOR_RED);
    lv_obj_align(menu->t2_right_label, LV_ALIGN_RIGHT_MID, -20, 0);
    y += ROW_NORMAL_HEIGHT + GRAD_HEIGHT;

    /* T1行 - 蓝色 */
    lv_obj_t * t1_row = create_row(menu->page1, ROW_NORMAL_HEIGHT, y);
    menu->t1_left_label  = create_label_bubble(t1_row, "(T1) 出口 °C", font_normal_32, COLOR_BLUE);
    lv_obj_align(menu->t1_left_label,  LV_ALIGN_LEFT_MID,  20,  0);
    menu->t1_right_label = create_label_bubble(t1_row, "--.-", font_normal_32, COLOR_BLUE);
    lv_obj_align(menu->t1_right_label, LV_ALIGN_RIGHT_MID, -20, 0);
    y += ROW_NORMAL_HEIGHT + GRAD_HEIGHT;

    /* 温度差显示区 - 绿色 */
    int16_t diff_h = MENU_HEIGHT - y - GRAD_HEIGHT;
    lv_obj_t * diff_row = create_row(menu->page1, diff_h, y);
    menu->temp_diff_title_label = create_label_bubble(diff_row, "温度差 °C", font_normal_32, COLOR_GREEN);
    lv_obj_align(menu->temp_diff_title_label, LV_ALIGN_TOP_LEFT,    20,  15);
    menu->temp_diff_value_label = create_label_bubble(diff_row, "--.-", font_char_72, COLOR_GREEN);
    lv_obj_align(menu->temp_diff_value_label, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
}

/* ------------------------------------------------------------------ */
/* Page2 - 详细数据页，三块均分                                          */
/* ------------------------------------------------------------------ */
static void create_page2(ui_temp_diff_menu_t * menu) {
    menu->page2 = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->page2);
    lv_obj_set_size(menu->page2, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_set_pos(menu->page2, 0, 0);
    lv_obj_clear_flag(menu->page2, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(menu->page2, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_EVENT_BUBBLE);

    int16_t y = 0;

    /* T2功能块 - 红色 (入口) */
    lv_obj_t * t2_row = create_row(menu->page2, PAGE2_BLOCK_HEIGHT, y);
    menu->page2_t2_pos_label   = create_label_bubble(t2_row, "(T2) 入口", font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_t2_pos_label,   LV_ALIGN_TOP_LEFT,    20,  15);
    menu->page2_t2_unit_label  = create_label_bubble(t2_row, "°C",   font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_t2_unit_label,  LV_ALIGN_TOP_RIGHT,   -20, 15);
    menu->page2_t2_icon        = create_thermometer_icon(t2_row, COLOR_RED);
    lv_obj_align(menu->page2_t2_icon,        LV_ALIGN_LEFT_MID,     30,  25);
    menu->page2_t2_value_label = create_label_bubble(t2_row, "--.-", font_char_72, COLOR_RED);
    lv_obj_align(menu->page2_t2_value_label, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
    y += PAGE2_BLOCK_HEIGHT + GRAD_HEIGHT;

    /* T1功能块 - 蓝色 (出口) */
    lv_obj_t * t1_row = create_row(menu->page2, PAGE2_BLOCK_HEIGHT, y);
    menu->page2_t1_pos_label   = create_label_bubble(t1_row, "(T1) 出口", font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_t1_pos_label,   LV_ALIGN_TOP_LEFT,    20,  15);
    menu->page2_t1_unit_label  = create_label_bubble(t1_row, "°C",   font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_t1_unit_label,  LV_ALIGN_TOP_RIGHT,   -20, 15);
    menu->page2_t1_icon        = create_thermometer_icon(t1_row, COLOR_BLUE);
    lv_obj_align(menu->page2_t1_icon,        LV_ALIGN_LEFT_MID,     30,  25);
    menu->page2_t1_value_label = create_label_bubble(t1_row, "--.-", font_char_72, COLOR_BLUE);
    lv_obj_align(menu->page2_t1_value_label, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
    y += PAGE2_BLOCK_HEIGHT + GRAD_HEIGHT;

    /* 温度差功能块 - 绿色 */
    int16_t diff_h = MENU_HEIGHT - y - GRAD_HEIGHT;   /* 用剩余高度，避免像素偏差 */
    lv_obj_t * diff_row = create_row(menu->page2, diff_h, y);
    menu->page2_diff_pos_label   = create_label_bubble(diff_row, "温度差", font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_diff_pos_label,   LV_ALIGN_TOP_LEFT,    20,  15);
    menu->page2_diff_unit_label  = create_label_bubble(diff_row, "°C",   font_normal_32, COLOR_TEXT_NORMAL);
    lv_obj_align(menu->page2_diff_unit_label,  LV_ALIGN_TOP_RIGHT,   -20, 15);
    menu->page2_diff_value_label = create_label_bubble(diff_row, "--.-", font_char_72, COLOR_GREEN);
    lv_obj_align(menu->page2_diff_value_label, LV_ALIGN_BOTTOM_RIGHT, -20, -15);
}

/* ------------------------------------------------------------------ */
/* 公开接口                                                             */
/* ------------------------------------------------------------------ */
ui_temp_diff_menu_t * ui_Temp_Diff_Menu_create(lv_obj_t * parent) {
    ui_temp_diff_menu_t * menu = (ui_temp_diff_menu_t *)lv_malloc(sizeof(ui_temp_diff_menu_t));
    if (menu == NULL) return NULL;
    memset(menu, 0, sizeof(ui_temp_diff_menu_t));

    menu->main_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(menu->main_cont);
    lv_obj_set_size(menu->main_cont, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->main_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(menu->main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->main_cont, LV_OBJ_FLAG_CLICKABLE);

    create_page1(menu);
    create_page2(menu);

    menu->total_pages = TOTAL_PAGES;
    show_page(menu, 0);

    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESSED,    menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_RELEASED,   menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESS_LOST, menu);

    return menu;
}

void ui_Temp_Diff_Menu_set_t1(ui_temp_diff_menu_t * menu, const char * value) {
    if (menu == NULL || value == NULL) return;
    if (menu->t1_right_label)       lv_label_set_text(menu->t1_right_label,       value);
    if (menu->page2_t1_value_label) lv_label_set_text(menu->page2_t1_value_label, value);
}

void ui_Temp_Diff_Menu_set_t2(ui_temp_diff_menu_t * menu, const char * value) {
    if (menu == NULL || value == NULL) return;
    if (menu->t2_right_label)       lv_label_set_text(menu->t2_right_label,       value);
    if (menu->page2_t2_value_label) lv_label_set_text(menu->page2_t2_value_label, value);
}

void ui_Temp_Diff_Menu_set_diff(ui_temp_diff_menu_t * menu, const char * value) {
    if (menu == NULL || value == NULL) return;
    if (menu->temp_diff_value_label)     lv_label_set_text(menu->temp_diff_value_label,     value);
    if (menu->page2_diff_value_label)    lv_label_set_text(menu->page2_diff_value_label,    value);
}

void ui_Temp_Diff_Menu_add_chart_point(ui_temp_diff_menu_t * menu, int32_t t1_value, int32_t t2_value) {
    if (menu == NULL || menu->chart == NULL) return;
    if (menu->chart_series_t1) lv_chart_set_next_value(menu->chart, menu->chart_series_t1, t1_value);
    if (menu->chart_series_t2) lv_chart_set_next_value(menu->chart, menu->chart_series_t2, t2_value);
    lv_chart_refresh(menu->chart);
}

void ui_Temp_Diff_Menu_set_chart_range(ui_temp_diff_menu_t * menu, int32_t min, int32_t max) {
    if (menu == NULL || menu->chart == NULL) return;
    lv_chart_set_range(menu->chart, LV_CHART_AXIS_PRIMARY_Y, min, max);
}

void ui_Temp_Diff_Menu_switch_page(ui_temp_diff_menu_t * menu, uint8_t page) {
    if (menu == NULL) return;
    show_page(menu, page % TOTAL_PAGES);
}

void ui_Temp_Diff_Menu_next_page(ui_temp_diff_menu_t * menu) {
    if (menu == NULL) return;
    show_page(menu, (menu->current_page + 1) % TOTAL_PAGES);
}

void ui_Temp_Diff_Menu_prev_page(ui_temp_diff_menu_t * menu) {
    if (menu == NULL) return;
    show_page(menu, (menu->current_page + TOTAL_PAGES - 1) % TOTAL_PAGES);
}

uint8_t ui_Temp_Diff_Menu_get_current_page(ui_temp_diff_menu_t * menu) {
    if (menu == NULL) return 0;
    return menu->current_page;
}

uint8_t ui_Temp_Diff_Menu_get_total_pages(ui_temp_diff_menu_t * menu) {
    if (menu == NULL) return 0;
    return menu->total_pages;
}

void ui_Temp_Diff_Menu_delete(ui_temp_diff_menu_t * menu) {
    if (menu == NULL) return;
    if (menu->main_cont) lv_obj_del(menu->main_cont);
    lv_free(menu);
}
