/**
 * @file ui_Pressure_Menu.c
 * @brief 压力测量界面
 *
 * 实现2页压力测量显示（数据列表页+图表页），支持手势滑动切换、
 * 峰值显示和数显区域。
 *
 * 优化策略:
 *   - 图表在创建时直接构建，无延迟加载
 *   - 渐变分隔线用绘制回调代替额外LVGL对象
 *   - 绝对定位代替flex布局，避免relayout
 *   - 所有子对象创建时直接设EVENT_BUBBLE
 *   - 图表数据用set_all_value批量初始化
 */

#include "measurement_ui.h"

#define MENU_WIDTH          480
#define MENU_HEIGHT         620
#define ROW1_HEIGHT         120
#define ROW_NORMAL_HEIGHT   60
#define GRAD_HEIGHT         5

#define COLOR_BG_NORMAL     0xD3D3D3
#define COLOR_TEXT_NORMAL   0x000000
#define COLOR_DISPLAY_VALUE 0x000080

#define TOTAL_PAGES         2
#define SWIPE_THRESHOLD     50

static lv_point_t press_point;
static bool is_pressing = false;

/* 渐变线绘制回调 */
static void row_draw_cb(lv_event_t * e) {
    if (lv_event_get_code(e) != LV_EVENT_DRAW_POST) return;

    lv_obj_t * obj = lv_event_get_target(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(obj, &a);

    int32_t y = a.y2 - GRAD_HEIGHT + 1;

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

    lv_area_t grad_area = {a.x1, y, a.x2, a.y2};
    lv_draw_rect(layer, &dsc, &grad_area);
}

/**
 * @brief 创建轻量行
 */
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
    if (font) lv_obj_set_style_text_font(label, font, 0);
    if (color) lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);
    return label;
}

static void show_page(ui_pressure_menu_t * menu, uint8_t page_index) {
    if (menu == NULL) return;

    if (menu->page1) lv_obj_add_flag(menu->page1, LV_OBJ_FLAG_HIDDEN);
    if (menu->page2) lv_obj_add_flag(menu->page2, LV_OBJ_FLAG_HIDDEN);

    switch (page_index) {
        case 0:
            if (menu->page1) lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_HIDDEN);
            break;
        case 1:
            if (menu->page2) lv_obj_clear_flag(menu->page2, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            if (menu->page1) lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_HIDDEN);
            break;
    }

    menu->current_page = page_index;
}

static void touch_event_cb(lv_event_t * e) {
    ui_pressure_menu_t * menu = (ui_pressure_menu_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (menu == NULL) return;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &press_point);
            is_pressing = true;
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!is_pressing) return;
        is_pressing = false;

        lv_indev_t * indev = lv_indev_get_act();
        if (indev == NULL) return;

        lv_point_t rp;
        lv_indev_get_point(indev, &rp);
        int32_t dx = rp.x - press_point.x;

        if (dx < -SWIPE_THRESHOLD) {
            show_page(menu, (menu->current_page + 1) % TOTAL_PAGES);
        } else if (dx > SWIPE_THRESHOLD) {
            show_page(menu, (menu->current_page + TOTAL_PAGES - 1) % TOTAL_PAGES);
        }
    }
}

/**
 * @brief 创建页面1 - 数据列表页
 */
static void create_page1(ui_pressure_menu_t * menu) {
    menu->page1 = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->page1);
    lv_obj_set_size(menu->page1, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_set_pos(menu->page1, 0, 0);
    lv_obj_clear_flag(menu->page1, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(menu->page1, LV_OBJ_FLAG_EVENT_BUBBLE);

    int16_t y = 0;

    /* 第1行: 单位行 */
    lv_obj_t * row1 = create_row(menu->page1, ROW1_HEIGHT, y);
    menu->unit_label = create_label_bubble(row1, "Pa", font_normal_32, 0);
    lv_obj_align(menu->unit_label, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    y += ROW1_HEIGHT + GRAD_HEIGHT;

    /* 第2-6行: 数据行 */
    const char * default_left[] = {"差压", "压力2", "压力3", "压力4", "pressure peak"};

    for (int i = 0; i < 5; i++) {
        lv_obj_t * row = create_row(menu->page1, ROW_NORMAL_HEIGHT, y);

        lv_obj_t * ll = create_label_bubble(row, default_left[i], font_normal_32, 0);
        lv_obj_align(ll, LV_ALIGN_LEFT_MID, 20, 0);

        lv_obj_t * rl = create_label_bubble(row, (i == 4) ? "0.00" : "0.0", font_normal_32, 0);
        lv_obj_align(rl, LV_ALIGN_RIGHT_MID, -20, 0);

        if (i < 4) {
            menu->row_labels[i][0] = ll;
            menu->row_labels[i][1] = rl;
        } else {
            menu->peak_left_label = ll;
            menu->peak_right_label = rl;
        }

        y += ROW_NORMAL_HEIGHT + GRAD_HEIGHT;
    }

    /* 第7行: 数显行 */
    int16_t display_h = MENU_HEIGHT - y - GRAD_HEIGHT;
    lv_obj_t * disp_row = create_row(menu->page1, display_h, y);

    lv_obj_t * dc = lv_obj_create(disp_row);
    lv_obj_remove_style_all(dc);
    lv_obj_set_size(dc, 200, display_h - 20);
    lv_obj_align(dc, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_clear_flag(dc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(dc, LV_OBJ_FLAG_EVENT_BUBBLE);

    menu->display_unit_label = create_label_bubble(dc, "Pa", font_normal_32, COLOR_DISPLAY_VALUE);
    lv_obj_align(menu->display_unit_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    menu->display_value_label = create_label_bubble(dc, "-0.7", font_char_72, COLOR_DISPLAY_VALUE);
    lv_obj_align(menu->display_value_label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

/**
 * @brief 创建页面2 - 图表页
 */
static void create_page2(ui_pressure_menu_t * menu) {
    menu->page2 = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->page2);
    lv_obj_set_size(menu->page2, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_set_pos(menu->page2, 0, 0);
    lv_obj_clear_flag(menu->page2, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(menu->page2, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_EVENT_BUBBLE);

    int16_t y = 0;

    /* 图表区域 */
    int16_t chart_h = (ROW1_HEIGHT + GRAD_HEIGHT) + (ROW_NORMAL_HEIGHT + GRAD_HEIGHT) * 4 - GRAD_HEIGHT;
    lv_obj_t * chart_row = create_row(menu->page2, chart_h, y);

    menu->chart = lv_chart_create(chart_row);
    lv_obj_set_size(menu->chart, MENU_WIDTH - 80, chart_h - 40);
    lv_obj_center(menu->chart);
    lv_obj_add_flag(menu->chart, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(menu->chart, LV_OBJ_FLAG_CLICKABLE);

    lv_chart_set_type(menu->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(menu->chart, 100);
    lv_chart_set_range(menu->chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);
    lv_chart_set_div_line_count(menu->chart, 3, 5);

    lv_obj_set_style_bg_color(menu->chart, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(menu->chart, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(menu->chart, 1, 0);
    lv_obj_set_style_line_color(menu->chart, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_line_width(menu->chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(menu->chart, 0, 0, LV_PART_INDICATOR);

    menu->chart_series = lv_chart_add_series(menu->chart, lv_color_hex(0x000000), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(menu->chart, menu->chart_series, 0);

    /* Y轴刻度 */
    lv_obj_t * sl;
    sl = create_label_bubble(chart_row, "1", &lv_font_montserrat_16, 0);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_TOP, -5, 0);
    sl = create_label_bubble(chart_row, "0", &lv_font_montserrat_16, 0);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    sl = create_label_bubble(chart_row, "-1", &lv_font_montserrat_16, 0);
    lv_obj_align_to(sl, menu->chart, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 0);

    y += chart_h + GRAD_HEIGHT;

    /* pressure peak行 */
    lv_obj_t * peak_row = create_row(menu->page2, ROW_NORMAL_HEIGHT, y);
    menu->page2_peak_left = create_label_bubble(peak_row, "pressure peak", font_normal_32, 0);
    lv_obj_align(menu->page2_peak_left, LV_ALIGN_LEFT_MID, 20, 0);
    menu->page2_peak_right = create_label_bubble(peak_row, "0.00", font_normal_32, 0);
    lv_obj_align(menu->page2_peak_right, LV_ALIGN_RIGHT_MID, -20, 0);
    y += ROW_NORMAL_HEIGHT + GRAD_HEIGHT;

    /* 数显行 */
    int16_t display_h = MENU_HEIGHT - y - GRAD_HEIGHT;
    lv_obj_t * disp_row = create_row(menu->page2, display_h, y);

    lv_obj_t * dc = lv_obj_create(disp_row);
    lv_obj_remove_style_all(dc);
    lv_obj_set_size(dc, 200, display_h - 20);
    lv_obj_align(dc, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_clear_flag(dc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(dc, LV_OBJ_FLAG_EVENT_BUBBLE);

    menu->page2_unit_label = create_label_bubble(dc, "Pa", font_normal_32, COLOR_DISPLAY_VALUE);
    lv_obj_align(menu->page2_unit_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    menu->page2_value_label = create_label_bubble(dc, "-0.7", font_char_72, COLOR_DISPLAY_VALUE);
    lv_obj_align(menu->page2_value_label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

/**
 * @brief 创建压力测量界面
 */
ui_pressure_menu_t * ui_Pressure_Menu_create(lv_obj_t * parent) {
    ui_pressure_menu_t * menu = (ui_pressure_menu_t *)lv_malloc(sizeof(ui_pressure_menu_t));
    if (menu == NULL) return NULL;
    memset(menu, 0, sizeof(ui_pressure_menu_t));

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

    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESSED, menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_RELEASED, menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESS_LOST, menu);

    return menu;
}

void ui_Pressure_Menu_set_row(ui_pressure_menu_t * menu, uint8_t row_index,
                               const char * left_text, const char * right_text) {
    if (menu == NULL || row_index >= 4) return;
    if (left_text && menu->row_labels[row_index][0]) lv_label_set_text(menu->row_labels[row_index][0], left_text);
    if (right_text && menu->row_labels[row_index][1]) lv_label_set_text(menu->row_labels[row_index][1], right_text);
}

void ui_Pressure_Menu_set_peak(ui_pressure_menu_t * menu, const char * value) {
    if (menu == NULL || value == NULL) return;
    if (menu->peak_right_label) lv_label_set_text(menu->peak_right_label, value);
    if (menu->page2_peak_right) lv_label_set_text(menu->page2_peak_right, value);
}

void ui_Pressure_Menu_set_display(ui_pressure_menu_t * menu, const char * unit, const char * value) {
    if (menu == NULL) return;
    if (unit) {
        if (menu->display_unit_label) lv_label_set_text(menu->display_unit_label, unit);
        if (menu->page2_unit_label) lv_label_set_text(menu->page2_unit_label, unit);
    }
    if (value) {
        if (menu->display_value_label) lv_label_set_text(menu->display_value_label, value);
        if (menu->page2_value_label) lv_label_set_text(menu->page2_value_label, value);
    }
}

void ui_Pressure_Menu_add_chart_point(ui_pressure_menu_t * menu, int32_t value) {
    if (menu == NULL || menu->chart == NULL || menu->chart_series == NULL) return;
    lv_chart_set_next_value(menu->chart, menu->chart_series, value);
    lv_chart_refresh(menu->chart);
}

void ui_Pressure_Menu_switch_page(ui_pressure_menu_t * menu, uint8_t page) {
    if (menu == NULL) return;
    show_page(menu, page % TOTAL_PAGES);
}

void ui_Pressure_Menu_next_page(ui_pressure_menu_t * menu) {
    if (menu == NULL) return;
    show_page(menu, (menu->current_page + 1) % TOTAL_PAGES);
}

void ui_Pressure_Menu_prev_page(ui_pressure_menu_t * menu) {
    if (menu == NULL) return;
    show_page(menu, (menu->current_page + TOTAL_PAGES - 1) % TOTAL_PAGES);
}

uint8_t ui_Pressure_Menu_get_current_page(ui_pressure_menu_t * menu) {
    if (menu == NULL) return 0;
    return menu->current_page;
}

uint8_t ui_Pressure_Menu_get_total_pages(ui_pressure_menu_t * menu) {
    if (menu == NULL) return 0;
    return menu->total_pages;
}

void ui_Pressure_Menu_delete(ui_pressure_menu_t * menu) {
    if (menu == NULL) return;
    if (menu->main_cont) lv_obj_del(menu->main_cont);
    lv_free(menu);
}
