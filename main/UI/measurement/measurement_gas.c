/**
 * @file ui_Gas_Menu.c
 * @brief 烟气测量显示界面
 *
 * 实现5页烟气测量数据显示（A/B/C/D/E页面），支持手势滑动切换、
 * 数值/图表显示、延迟加载图表以优化内存。
 */

#include "measurement_ui.h"

#define TAG "ui_gas_display"

#define MENU_WIDTH          480
#define MENU_HEIGHT         620

#define GRAD_HEIGHT         5

#define COLOR_BG_NORMAL     0xD3D3D3
#define COLOR_TEXT_NORMAL   0x000000
#define COLOR_VALUE_DEFAULT 0x000000
#define COLOR_VALUE_ORANGE  0xFF8C00

/* 页面数量 */
#define PAGE_COUNT          5
#define PAGE_A              0   /* 主界面 */
#define PAGE_B              1   /* 上界面 */
#define PAGE_C              2   /* 下界面 */
#define PAGE_D              3   /* 左界面 */
#define PAGE_E              4   /* 右界面 */

/* 滑动检测 */
#define SWIPE_THRESHOLD     50

static lv_point_t press_point;
static bool is_pressing = false;
static bool g_page_b_initialized = false;

/* 内部函数声明 */
static lv_obj_t * create_row_with_gradient(lv_obj_t * parent, int16_t height);
static lv_obj_t * get_item_from_wrapper(lv_obj_t * wrapper);
static void set_event_bubble_recursive(lv_obj_t * obj);
static void touch_event_cb(lv_event_t * e);

static void create_page_a(ui_gas_measure_display_t * menu);
static void create_page_b(ui_gas_measure_display_t * menu);
static void create_page_c(ui_gas_measure_display_t * menu);
static void create_page_d(ui_gas_measure_display_t * menu);
static void create_page_e(ui_gas_measure_display_t * menu);
static void lazy_create_page_b_charts(ui_gas_measure_display_t * menu);

static void show_page(ui_gas_measure_display_t * menu, uint8_t page_index);
static void hide_all_pages(ui_gas_measure_display_t * menu);

/**
 * @brief 递归设置事件冒泡
 */
static void set_event_bubble_recursive(lv_obj_t * obj) {
    if (obj == NULL) return;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    uint32_t child_count = lv_obj_get_child_count(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        set_event_bubble_recursive(lv_obj_get_child(obj, i));
    }
}

/**
 * @brief 创建带渐变分隔线的行
 */
static lv_obj_t * create_row_with_gradient(lv_obj_t * parent, int16_t height) {
    lv_obj_t * wrapper = lv_obj_create(parent);
    lv_obj_remove_style_all(wrapper);
    lv_obj_set_size(wrapper, MENU_WIDTH, height + GRAD_HEIGHT);
    lv_obj_clear_flag(wrapper, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * item = lv_obj_create(wrapper);
    lv_obj_set_size(item, MENU_WIDTH, height);
    lv_obj_align(item, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, 0, 0);
    lv_obj_set_style_bg_color(item, lv_color_hex(COLOR_BG_NORMAL), 0);
    lv_obj_set_style_text_color(item, lv_color_hex(COLOR_TEXT_NORMAL), 0);

    lv_obj_t * grad = lv_obj_create(wrapper);
    lv_obj_remove_style_all(grad);
    lv_obj_set_size(grad, MENU_WIDTH, GRAD_HEIGHT);
    lv_obj_align(grad, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_set_style_bg_opa(grad, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(grad, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_grad_color(grad, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_dir(grad, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_main_stop(grad, 88, 0);
    lv_obj_set_style_bg_grad_stop(grad, 255, 0);

    return wrapper;
}

static lv_obj_t * get_item_from_wrapper(lv_obj_t * wrapper) {
    return lv_obj_get_child(wrapper, 0);
}

/**
 * @brief 隐藏所有页面
 */
static void hide_all_pages(ui_gas_measure_display_t * menu) {
    if (menu == NULL) return;

    for (int i = 0; i < PAGE_COUNT; i++) {
        if (menu->pages[i] != NULL) {
            lv_obj_add_flag(menu->pages[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/**
 * @brief 显示指定页面
 */
static void show_page(ui_gas_measure_display_t * menu, uint8_t page_index) {
    if (menu == NULL || page_index >= PAGE_COUNT) return;

    hide_all_pages(menu);

    if (page_index == PAGE_B && !g_page_b_initialized) {
        lazy_create_page_b_charts(menu);
    }

    if (menu->pages[page_index] != NULL) {
        lv_obj_clear_flag(menu->pages[page_index], LV_OBJ_FLAG_HIDDEN);
    }

    menu->current_page = page_index;
}

/**
 * @brief 触摸事件回调 - 支持上下左右循环滑动
 */
static void touch_event_cb(lv_event_t * e) {
    ui_gas_measure_display_t * menu = (ui_gas_measure_display_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (menu == NULL) return;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev != NULL) {
            lv_indev_get_point(indev, &press_point);
            is_pressing = true;
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!is_pressing) return;
        is_pressing = false;

        lv_indev_t * indev = lv_indev_get_act();
        if (indev == NULL) return;

        lv_point_t release_point;
        lv_indev_get_point(indev, &release_point);

        int32_t diff_x = release_point.x - press_point.x;
        int32_t diff_y = release_point.y - press_point.y;

        /* 判断滑动方向 - 取绝对值较大的方向 */
        if (abs(diff_x) > abs(diff_y)) {
            /* 水平滑动 */
            if (diff_x < -SWIPE_THRESHOLD) {
                /* 向左滑 - 从A到E，或从D到A，或从E到D（循环） */
                if (menu->current_page == PAGE_A) {
                    show_page(menu, PAGE_E);
                } else if (menu->current_page == PAGE_D) {
                    show_page(menu, PAGE_A);
                } else if (menu->current_page == PAGE_E) {
                    show_page(menu, PAGE_D);
                }
            }
            else if (diff_x > SWIPE_THRESHOLD) {
                /* 向右滑 - 从A到D，或从E到A，或从D到E（循环） */
                if (menu->current_page == PAGE_A) {
                    show_page(menu, PAGE_D);
                } else if (menu->current_page == PAGE_E) {
                    show_page(menu, PAGE_A);
                } else if (menu->current_page == PAGE_D) {
                    show_page(menu, PAGE_E);
                }
            }
        } else {
            /* 垂直滑动 */
            if (diff_y < -SWIPE_THRESHOLD) {
                /* 向上滑 - 从A到C，或从B到A，或从C到B（循环） */
                if (menu->current_page == PAGE_A) {
                    show_page(menu, PAGE_C);
                } else if (menu->current_page == PAGE_B) {
                    show_page(menu, PAGE_A);
                } else if (menu->current_page == PAGE_C) {
                    show_page(menu, PAGE_B);
                }
            }
            else if (diff_y > SWIPE_THRESHOLD) {
                /* 向下滑 - 从A到B，或从C到A，或从B到C（循环） */
                if (menu->current_page == PAGE_A) {
                    show_page(menu, PAGE_B);
                } else if (menu->current_page == PAGE_C) {
                    show_page(menu, PAGE_A);
                } else if (menu->current_page == PAGE_B) {
                    show_page(menu, PAGE_C);
                }
            }
        }
    }
}

/**
 * @brief 创建页面A - 主界面，7个功能块，左侧参数+单位，右侧数值
 * 布局: 700px / 7 = 100px each
 */
static void create_page_a(ui_gas_measure_display_t * menu) {
    menu->pages[PAGE_A] = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->pages[PAGE_A]);
    lv_obj_set_size(menu->pages[PAGE_A], MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->pages[PAGE_A], LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(menu->pages[PAGE_A], LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(menu->pages[PAGE_A], LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu->pages[PAGE_A], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu->pages[PAGE_A], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu->pages[PAGE_A], 0, 0);

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 7) / 7;  /* 约95px */

    for (int i = 0; i < 7; i++) {
        lv_obj_t * wrapper = create_row_with_gradient(menu->pages[PAGE_A], block_height);
        lv_obj_t * item = get_item_from_wrapper(wrapper);

        /* 左上 - 参数名 */
        menu->page_a_blocks[i].name_label = lv_label_create(item);
        lv_label_set_text(menu->page_a_blocks[i].name_label, "---");
        lv_obj_set_style_text_font(menu->page_a_blocks[i].name_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_a_blocks[i].name_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_a_blocks[i].name_label, LV_ALIGN_TOP_LEFT, 15, 8);

        /* 左下 - 单位 */
        menu->page_a_blocks[i].unit_label = lv_label_create(item);
        lv_label_set_text(menu->page_a_blocks[i].unit_label, "---");
        lv_obj_set_style_text_font(menu->page_a_blocks[i].unit_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_a_blocks[i].unit_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_a_blocks[i].unit_label, LV_ALIGN_BOTTOM_LEFT, 15, -8);

        /* 右侧 - 数值 */
        menu->page_a_blocks[i].value_label = lv_label_create(item);
        lv_label_set_text(menu->page_a_blocks[i].value_label, "--.-");
        lv_obj_set_style_text_font(menu->page_a_blocks[i].value_label, font_char_72, 0);
        lv_obj_set_style_text_color(menu->page_a_blocks[i].value_label, lv_color_hex(COLOR_VALUE_DEFAULT), 0);
        lv_obj_align(menu->page_a_blocks[i].value_label, LV_ALIGN_RIGHT_MID, -15, 0);
    }
}

/**
 * @brief 创建页面B - 上界面，4个功能块，左侧图表+中间名字单位+右侧数值
 * 布局: 700px / 4 = 175px each
 */
static void create_page_b(ui_gas_measure_display_t * menu) {
    menu->pages[PAGE_B] = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->pages[PAGE_B]);
    lv_obj_set_size(menu->pages[PAGE_B], MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->pages[PAGE_B], LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(menu->pages[PAGE_B], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->pages[PAGE_B], LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_layout(menu->pages[PAGE_B], LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu->pages[PAGE_B], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu->pages[PAGE_B], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu->pages[PAGE_B], 0, 0);

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 4) / 4;

    for (int i = 0; i < 4; i++) {
        lv_obj_t * wrapper = create_row_with_gradient(menu->pages[PAGE_B], block_height);
        lv_obj_t * item = get_item_from_wrapper(wrapper);

        /* 只创建文本标签，图表延迟创建 */
        menu->page_b_blocks[i].chart = NULL;
        menu->page_b_blocks[i].chart_series = NULL;

        /* 占位符标签 - 稍后图表会覆盖这个位置 */
        lv_obj_t * placeholder = lv_obj_create(item);
        lv_obj_remove_style_all(placeholder);
        lv_obj_set_size(placeholder, 130, block_height - 30);
        lv_obj_align(placeholder, LV_ALIGN_LEFT_MID, 30, 0);
        lv_obj_set_style_bg_color(placeholder, lv_color_hex(0xEEEEEE), 0);
        lv_obj_set_style_bg_opa(placeholder, LV_OPA_COVER, 0);
        lv_obj_set_user_data(item, placeholder);  /* 保存占位符引用 */

        /* 刻度标签 */
        #define SCALE_LABEL_WIDTH  25

        menu->page_b_blocks[i].scale_max_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].scale_max_label, "100");
        lv_obj_set_size(menu->page_b_blocks[i].scale_max_label, SCALE_LABEL_WIDTH, LV_SIZE_CONTENT);
        lv_obj_set_style_text_font(menu->page_b_blocks[i].scale_max_label, &lv_font_montserrat_10, 0);
        lv_obj_align(menu->page_b_blocks[i].scale_max_label, LV_ALIGN_LEFT_MID, 2, -(block_height/2 - 20));

        menu->page_b_blocks[i].scale_mid_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].scale_mid_label, "50");
        lv_obj_set_size(menu->page_b_blocks[i].scale_mid_label, SCALE_LABEL_WIDTH, LV_SIZE_CONTENT);
        lv_obj_set_style_text_font(menu->page_b_blocks[i].scale_mid_label, &lv_font_montserrat_10, 0);
        lv_obj_align(menu->page_b_blocks[i].scale_mid_label, LV_ALIGN_LEFT_MID, 2, 0);

        menu->page_b_blocks[i].scale_min_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].scale_min_label, "0");
        lv_obj_set_size(menu->page_b_blocks[i].scale_min_label, SCALE_LABEL_WIDTH, LV_SIZE_CONTENT);
        lv_obj_set_style_text_font(menu->page_b_blocks[i].scale_min_label, &lv_font_montserrat_10, 0);
        lv_obj_align(menu->page_b_blocks[i].scale_min_label, LV_ALIGN_LEFT_MID, 2, (block_height/2 - 20));

        /* 名字和单位 */
        menu->page_b_blocks[i].name_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].name_label, "T-烟气");
        lv_obj_set_style_text_font(menu->page_b_blocks[i].name_label, font_normal_32, 0);
        lv_obj_align(menu->page_b_blocks[i].name_label, LV_ALIGN_TOP_LEFT, 170, 15);

        menu->page_b_blocks[i].unit_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].unit_label, "°C");
        lv_obj_set_style_text_font(menu->page_b_blocks[i].unit_label, font_normal_32, 0);
        lv_obj_align(menu->page_b_blocks[i].unit_label, LV_ALIGN_BOTTOM_LEFT, 170, -15);

        /* 数值 */
        menu->page_b_blocks[i].value_label = lv_label_create(item);
        lv_label_set_text(menu->page_b_blocks[i].value_label, "--.-");
        lv_obj_set_style_text_font(menu->page_b_blocks[i].value_label, font_char_72, 0);
        lv_obj_align(menu->page_b_blocks[i].value_label, LV_ALIGN_RIGHT_MID, -15, 0);
    }

    g_page_b_initialized = false;
}

/* 延迟创建图表的函数 */
static void lazy_create_page_b_charts(ui_gas_measure_display_t * menu) {
    if (menu == NULL || g_page_b_initialized) return;

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 4) / 4;

    for (int i = 0; i < 4; i++) {
        lv_obj_t * wrapper = lv_obj_get_child(menu->pages[PAGE_B], i);
        if (wrapper == NULL) continue;

        lv_obj_t * item = lv_obj_get_child(wrapper, 0);
        if (item == NULL) continue;

        /* 删除占位符 */
        lv_obj_t * placeholder = (lv_obj_t *)lv_obj_get_user_data(item);
        if (placeholder) {
            lv_obj_del(placeholder);
            lv_obj_set_user_data(item, NULL);
        }

        /* 创建实际图表 - 使用简化配置 */
        menu->page_b_blocks[i].chart = lv_chart_create(item);
        lv_obj_set_size(menu->page_b_blocks[i].chart, 130, block_height - 30);
        lv_obj_align(menu->page_b_blocks[i].chart, LV_ALIGN_LEFT_MID, 30, 0);

        lv_chart_set_type(menu->page_b_blocks[i].chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(menu->page_b_blocks[i].chart, 25);  /* 减少数据点 */
        lv_chart_set_range(menu->page_b_blocks[i].chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);

        /* 极简样式 */
        lv_obj_set_style_bg_color(menu->page_b_blocks[i].chart, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(menu->page_b_blocks[i].chart, 1, 0);
        lv_obj_set_style_border_color(menu->page_b_blocks[i].chart, lv_color_hex(0x000000), 0);
        lv_obj_set_style_line_width(menu->page_b_blocks[i].chart, 2, LV_PART_ITEMS);
        lv_obj_set_style_size(menu->page_b_blocks[i].chart, 0, 0, LV_PART_INDICATOR);
        lv_chart_set_div_line_count(menu->page_b_blocks[i].chart, 2, 3);  /* 减少分割线 */

        /* 颜色 */
        lv_color_t series_color;
        if (i == 0) series_color = lv_color_hex(0xFF0000);
        else if (i == 1) series_color = lv_color_hex(0x0000FF);
        else if (i == 2) series_color = lv_color_hex(0xFF0000);
        else series_color = lv_color_hex(0xFFA500);

        menu->page_b_blocks[i].chart_series = lv_chart_add_series(
            menu->page_b_blocks[i].chart, series_color, LV_CHART_AXIS_PRIMARY_Y);

        /* 使用 set_all_value 快速初始化 */
        lv_chart_set_all_value(menu->page_b_blocks[i].chart,
                               menu->page_b_blocks[i].chart_series, 50);
    }

    g_page_b_initialized = true;
}

/**
 * @brief 创建页面C - 下界面，2个功能块，左上名字+左下单位+右侧大数值
 * 布局: 700px / 2 = 350px each
 */
static void create_page_c(ui_gas_measure_display_t * menu) {
    menu->pages[PAGE_C] = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->pages[PAGE_C]);
    lv_obj_set_size(menu->pages[PAGE_C], MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->pages[PAGE_C], LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(menu->pages[PAGE_C], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->pages[PAGE_C], LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_layout(menu->pages[PAGE_C], LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu->pages[PAGE_C], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu->pages[PAGE_C], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu->pages[PAGE_C], 0, 0);

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 2) / 2;  /* 约345px */

    for (int i = 0; i < 2; i++) {
        lv_obj_t * wrapper = create_row_with_gradient(menu->pages[PAGE_C], block_height);
        lv_obj_t * item = get_item_from_wrapper(wrapper);

        /* 左上 - 名字 */
        menu->page_c_blocks[i].name_label = lv_label_create(item);
        lv_label_set_text(menu->page_c_blocks[i].name_label, "T-烟气");
        lv_obj_set_style_text_font(menu->page_c_blocks[i].name_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_c_blocks[i].name_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_c_blocks[i].name_label, LV_ALIGN_TOP_LEFT, 20, 20);

        /* 左下 - 单位 */
        menu->page_c_blocks[i].unit_label = lv_label_create(item);
        lv_label_set_text(menu->page_c_blocks[i].unit_label, "°C");
        lv_obj_set_style_text_font(menu->page_c_blocks[i].unit_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_c_blocks[i].unit_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_c_blocks[i].unit_label, LV_ALIGN_BOTTOM_LEFT, 20, -20);

        /* 右侧中央 - 大数值 */
        menu->page_c_blocks[i].value_label = lv_label_create(item);
        lv_label_set_text(menu->page_c_blocks[i].value_label, "--.-");
        lv_obj_set_style_text_font(menu->page_c_blocks[i].value_label, font_char_72, 0);
        lv_obj_set_style_text_color(menu->page_c_blocks[i].value_label, lv_color_hex(COLOR_VALUE_DEFAULT), 0);
        lv_obj_align(menu->page_c_blocks[i].value_label, LV_ALIGN_RIGHT_MID, -30, 0);
    }
}

/**
 * @brief 创建页面D - 左界面，7个功能块，布局同A
 */
static void create_page_d(ui_gas_measure_display_t * menu) {
    menu->pages[PAGE_D] = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->pages[PAGE_D]);
    lv_obj_set_size(menu->pages[PAGE_D], MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->pages[PAGE_D], LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(menu->pages[PAGE_D], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->pages[PAGE_D], LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_layout(menu->pages[PAGE_D], LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu->pages[PAGE_D], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu->pages[PAGE_D], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu->pages[PAGE_D], 0, 0);

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 7) / 7;

    for (int i = 0; i < 7; i++) {
        lv_obj_t * wrapper = create_row_with_gradient(menu->pages[PAGE_D], block_height);
        lv_obj_t * item = get_item_from_wrapper(wrapper);

        /* 左上 - 参数名 */
        menu->page_d_blocks[i].name_label = lv_label_create(item);
        lv_label_set_text(menu->page_d_blocks[i].name_label, "---");
        lv_obj_set_style_text_font(menu->page_d_blocks[i].name_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_d_blocks[i].name_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_d_blocks[i].name_label, LV_ALIGN_TOP_LEFT, 15, 8);

        /* 左下 - 单位 */
        menu->page_d_blocks[i].unit_label = lv_label_create(item);
        lv_label_set_text(menu->page_d_blocks[i].unit_label, "---");
        lv_obj_set_style_text_font(menu->page_d_blocks[i].unit_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_d_blocks[i].unit_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_d_blocks[i].unit_label, LV_ALIGN_BOTTOM_LEFT, 15, -8);

        /* 右侧 - 数值 */
        menu->page_d_blocks[i].value_label = lv_label_create(item);
        lv_label_set_text(menu->page_d_blocks[i].value_label, "--.-");
        lv_obj_set_style_text_font(menu->page_d_blocks[i].value_label, font_char_72, 0);
        lv_obj_set_style_text_color(menu->page_d_blocks[i].value_label, lv_color_hex(COLOR_VALUE_DEFAULT), 0);
        lv_obj_align(menu->page_d_blocks[i].value_label, LV_ALIGN_RIGHT_MID, -15, 0);
    }
}

/**
 * @brief 创建页面E - 右界面，7个功能块，布局同A
 */
static void create_page_e(ui_gas_measure_display_t * menu) {
    menu->pages[PAGE_E] = lv_obj_create(menu->main_cont);
    lv_obj_remove_style_all(menu->pages[PAGE_E]);
    lv_obj_set_size(menu->pages[PAGE_E], MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->pages[PAGE_E], LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(menu->pages[PAGE_E], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->pages[PAGE_E], LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_layout(menu->pages[PAGE_E], LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu->pages[PAGE_E], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu->pages[PAGE_E], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu->pages[PAGE_E], 0, 0);

    int16_t block_height = (MENU_HEIGHT - GRAD_HEIGHT * 7) / 7;

    for (int i = 0; i < 7; i++) {
        lv_obj_t * wrapper = create_row_with_gradient(menu->pages[PAGE_E], block_height);
        lv_obj_t * item = get_item_from_wrapper(wrapper);

        /* 左上 - 参数名 */
        menu->page_e_blocks[i].name_label = lv_label_create(item);
        lv_label_set_text(menu->page_e_blocks[i].name_label, "---");
        lv_obj_set_style_text_font(menu->page_e_blocks[i].name_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_e_blocks[i].name_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_e_blocks[i].name_label, LV_ALIGN_TOP_LEFT, 15, 8);

        /* 左下 - 单位 */
        menu->page_e_blocks[i].unit_label = lv_label_create(item);
        lv_label_set_text(menu->page_e_blocks[i].unit_label, "---");
        lv_obj_set_style_text_font(menu->page_e_blocks[i].unit_label, font_normal_32, 0);
        lv_obj_set_style_text_color(menu->page_e_blocks[i].unit_label, lv_color_hex(COLOR_TEXT_NORMAL), 0);
        lv_obj_align(menu->page_e_blocks[i].unit_label, LV_ALIGN_BOTTOM_LEFT, 15, -8);

        /* 右侧 - 数值 */
        menu->page_e_blocks[i].value_label = lv_label_create(item);
        lv_label_set_text(menu->page_e_blocks[i].value_label, "--.-");
        lv_obj_set_style_text_font(menu->page_e_blocks[i].value_label, font_char_72, 0);
        lv_obj_set_style_text_color(menu->page_e_blocks[i].value_label, lv_color_hex(COLOR_VALUE_DEFAULT), 0);
        lv_obj_align(menu->page_e_blocks[i].value_label, LV_ALIGN_RIGHT_MID, -15, 0);
    }
}

/* 公共API */

/**
 * @brief 创建烟气测量显示界面
 */
ui_gas_measure_display_t * ui_Gas_Measure_Display_create(lv_obj_t * parent) {
    ui_gas_measure_display_t * menu = (ui_gas_measure_display_t *)lv_malloc(sizeof(ui_gas_measure_display_t));
    if (menu == NULL) return NULL;

    memset(menu, 0, sizeof(ui_gas_measure_display_t));

    menu->main_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(menu->main_cont);
    lv_obj_set_size(menu->main_cont, MENU_WIDTH, MENU_HEIGHT);
    lv_obj_align(menu->main_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(menu->main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(menu->main_cont, LV_OBJ_FLAG_CLICKABLE);

    /* 创建所有页面 */
    create_page_a(menu);
    create_page_b(menu);
    create_page_c(menu);
    create_page_d(menu);
    create_page_e(menu);

    menu->current_page = PAGE_A;

    /* 设置事件冒泡 */
    for (int i = 0; i < PAGE_COUNT; i++) {
        set_event_bubble_recursive(menu->pages[i]);
    }

    /* 添加触摸事件 */
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESSED, menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_RELEASED, menu);
    lv_obj_add_event_cb(menu->main_cont, touch_event_cb, LV_EVENT_PRESS_LOST, menu);

    return menu;
}

/**
 * @brief 设置页面A的功能块内容
 * @param block_index 功能块索引 (0-6)
 * @param name 参数名
 * @param unit 单位
 * @param value 数值
 * @param value_color 数值颜色 (0表示默认黑色)
 */
/**
 * @brief 设置页面A功能块内容
 */
void ui_Gas_Measure_Display_set_page_a(ui_gas_measure_display_t * menu,
                                        uint8_t block_index,
                                        const char * name,
                                        const char * unit,
                                        const char * value,
                                        uint32_t value_color) {
    if (menu == NULL || block_index >= 7) return;

    if (name != NULL && menu->page_a_blocks[block_index].name_label != NULL) {
        lv_label_set_text(menu->page_a_blocks[block_index].name_label, name);
    }
    if (unit != NULL && menu->page_a_blocks[block_index].unit_label != NULL) {
        lv_label_set_text(menu->page_a_blocks[block_index].unit_label, unit);
    }
    if (value != NULL && menu->page_a_blocks[block_index].value_label != NULL) {
        lv_label_set_text(menu->page_a_blocks[block_index].value_label, value);
    }

    /* 设置数值颜色 */
    if (menu->page_a_blocks[block_index].value_label != NULL) {
        uint32_t color = (value_color == 0) ? COLOR_VALUE_DEFAULT : value_color;
        lv_obj_set_style_text_color(menu->page_a_blocks[block_index].value_label,
                                     lv_color_hex(color), 0);
    }
}

/**
 * @brief 设置页面B的功能块内容
 * @param block_index 功能块索引 (0-3)
 */
void ui_Gas_Measure_Display_set_page_b(ui_gas_measure_display_t * menu,
                                        uint8_t block_index,
                                        const char * name,
                                        const char * unit,
                                        const char * value,
                                        uint32_t value_color) {
    if (menu == NULL || block_index >= 4) return;

    if (name != NULL && menu->page_b_blocks[block_index].name_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].name_label, name);
    }
    if (unit != NULL && menu->page_b_blocks[block_index].unit_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].unit_label, unit);
    }
    if (value != NULL && menu->page_b_blocks[block_index].value_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].value_label, value);
    }

    if (menu->page_b_blocks[block_index].value_label != NULL) {
        uint32_t color = (value_color == 0) ? COLOR_VALUE_DEFAULT : value_color;
        lv_obj_set_style_text_color(menu->page_b_blocks[block_index].value_label,
                                     lv_color_hex(color), 0);
    }
}

/**
 * @brief 设置页面C的功能块内容
 * @param block_index 功能块索引 (0-1)
 */
void ui_Gas_Measure_Display_set_page_c(ui_gas_measure_display_t * menu,
                                        uint8_t block_index,
                                        const char * name,
                                        const char * unit,
                                        const char * value,
                                        uint32_t value_color) {
    if (menu == NULL || block_index >= 2) return;

    if (name != NULL && menu->page_c_blocks[block_index].name_label != NULL) {
        lv_label_set_text(menu->page_c_blocks[block_index].name_label, name);
    }
    if (unit != NULL && menu->page_c_blocks[block_index].unit_label != NULL) {
        lv_label_set_text(menu->page_c_blocks[block_index].unit_label, unit);
    }
    if (value != NULL && menu->page_c_blocks[block_index].value_label != NULL) {
        lv_label_set_text(menu->page_c_blocks[block_index].value_label, value);
    }

    if (menu->page_c_blocks[block_index].value_label != NULL) {
        uint32_t color = (value_color == 0) ? COLOR_VALUE_DEFAULT : value_color;
        lv_obj_set_style_text_color(menu->page_c_blocks[block_index].value_label,
                                     lv_color_hex(color), 0);
    }
}

/**
 * @brief 设置页面D的功能块内容
 * @param block_index 功能块索引 (0-6)
 */
void ui_Gas_Measure_Display_set_page_d(ui_gas_measure_display_t * menu,
                                        uint8_t block_index,
                                        const char * name,
                                        const char * unit,
                                        const char * value,
                                        uint32_t value_color) {
    if (menu == NULL || block_index >= 7) return;

    if (name != NULL && menu->page_d_blocks[block_index].name_label != NULL) {
        lv_label_set_text(menu->page_d_blocks[block_index].name_label, name);
    }
    if (unit != NULL && menu->page_d_blocks[block_index].unit_label != NULL) {
        lv_label_set_text(menu->page_d_blocks[block_index].unit_label, unit);
    }
    if (value != NULL && menu->page_d_blocks[block_index].value_label != NULL) {
        lv_label_set_text(menu->page_d_blocks[block_index].value_label, value);
    }

    if (menu->page_d_blocks[block_index].value_label != NULL) {
        uint32_t color = (value_color == 0) ? COLOR_VALUE_DEFAULT : value_color;
        lv_obj_set_style_text_color(menu->page_d_blocks[block_index].value_label,
                                     lv_color_hex(color), 0);
    }
}

/**
 * @brief 设置页面E的功能块内容
 * @param block_index 功能块索引 (0-6)
 */
void ui_Gas_Measure_Display_set_page_e(ui_gas_measure_display_t * menu,
                                        uint8_t block_index,
                                        const char * name,
                                        const char * unit,
                                        const char * value,
                                        uint32_t value_color) {
    if (menu == NULL || block_index >= 7) return;

    if (name != NULL && menu->page_e_blocks[block_index].name_label != NULL) {
        lv_label_set_text(menu->page_e_blocks[block_index].name_label, name);
    }
    if (unit != NULL && menu->page_e_blocks[block_index].unit_label != NULL) {
        lv_label_set_text(menu->page_e_blocks[block_index].unit_label, unit);
    }
    if (value != NULL && menu->page_e_blocks[block_index].value_label != NULL) {
        lv_label_set_text(menu->page_e_blocks[block_index].value_label, value);
    }

    if (menu->page_e_blocks[block_index].value_label != NULL) {
        uint32_t color = (value_color == 0) ? COLOR_VALUE_DEFAULT : value_color;
        lv_obj_set_style_text_color(menu->page_e_blocks[block_index].value_label,
                                     lv_color_hex(color), 0);
    }
}

/**
 * @brief 通用设置接口 - 设置任意页面任意功能块的内容
 * @param page 页面 ('A'-'E')
 * @param block_index 功能块索引
 * @param name 参数名 (可为NULL)
 * @param unit 单位 (可为NULL)
 * @param value 数值 (可为NULL)
 * @param value_color 数值颜色 (0表示默认黑色)
 */
/**
 * @brief 通用设置接口(按页面标识)
 */
void ui_Gas_Measure_Display_set_block(ui_gas_measure_display_t * menu,
                                       char page,
                                       uint8_t block_index,
                                       const char * name,
                                       const char * unit,
                                       const char * value,
                                       uint32_t value_color) {
    if (menu == NULL) return;

    switch (page) {
        case 'A':
        case 'a':
            ui_Gas_Measure_Display_set_page_a(menu, block_index, name, unit, value, value_color);
            break;
        case 'B':
        case 'b':
            ui_Gas_Measure_Display_set_page_b(menu, block_index, name, unit, value, value_color);
            break;
        case 'C':
        case 'c':
            ui_Gas_Measure_Display_set_page_c(menu, block_index, name, unit, value, value_color);
            break;
        case 'D':
        case 'd':
            ui_Gas_Measure_Display_set_page_d(menu, block_index, name, unit, value, value_color);
            break;
        case 'E':
        case 'e':
            ui_Gas_Measure_Display_set_page_e(menu, block_index, name, unit, value, value_color);
            break;
        default:
            break;
    }
}

/**
 * @brief 仅设置数值和颜色（快速更新接口）
 */
void ui_Gas_Measure_Display_set_value(ui_gas_measure_display_t * menu,
                                       char page,
                                       uint8_t block_index,
                                       const char * value,
                                       uint32_t value_color) {
    ui_Gas_Measure_Display_set_block(menu, page, block_index, NULL, NULL, value, value_color);
}

/**
 * @brief 为页面B的图表添加数据点
 */
void ui_Gas_Measure_Display_add_chart_point_b(ui_gas_measure_display_t * menu,
                                               uint8_t block_index,
                                               int32_t value) {
    if (menu == NULL || block_index >= 4) return;

    if (menu->page_b_blocks[block_index].chart != NULL &&
        menu->page_b_blocks[block_index].chart_series != NULL) {
        lv_chart_set_next_value(menu->page_b_blocks[block_index].chart,
                                menu->page_b_blocks[block_index].chart_series, value);
        lv_chart_refresh(menu->page_b_blocks[block_index].chart);
    }
}

/**
 * @brief 设置页面B图表的Y轴范围
 */
void ui_Gas_Measure_Display_set_chart_range_b(ui_gas_measure_display_t * menu,
                                               uint8_t block_index,
                                               int32_t min,
                                               int32_t max) {
    if (menu == NULL || block_index >= 4) return;

    if (menu->page_b_blocks[block_index].chart != NULL) {
        lv_chart_set_range(menu->page_b_blocks[block_index].chart,
                           LV_CHART_AXIS_PRIMARY_Y, min, max);
    }
}

/**
 * @brief 切换到指定页面
 */
void ui_Gas_Measure_Display_switch_page(ui_gas_measure_display_t * menu, char page) {
    if (menu == NULL) return;

    uint8_t page_index = PAGE_A;

    switch (page) {
        case 'A': case 'a': page_index = PAGE_A; break;
        case 'B': case 'b': page_index = PAGE_B; break;
        case 'C': case 'c': page_index = PAGE_C; break;
        case 'D': case 'd': page_index = PAGE_D; break;
        case 'E': case 'e': page_index = PAGE_E; break;
        default: return;
    }

    show_page(menu, page_index);
}

/**
 * @brief 获取当前页面
 */
char ui_Gas_Measure_Display_get_current_page(ui_gas_measure_display_t * menu) {
    if (menu == NULL) return 'A';

    const char pages[] = {'A', 'B', 'C', 'D', 'E'};
    if (menu->current_page < PAGE_COUNT) {
        return pages[menu->current_page];
    }
    return 'A';
}

/**
 * @brief 删除界面
 */
void ui_Gas_Measure_Display_delete(ui_gas_measure_display_t * menu) {
    if (menu == NULL) return;

    g_page_b_initialized = false;

    if (menu->main_cont != NULL) {
        lv_obj_del(menu->main_cont);
    }

    lv_free(menu);
}

/**
 * @brief 设置页面B图表的刻度标签
 * @param block_index 功能块索引 (0-3)
 * @param max_val 最大值刻度文本
 * @param mid_val 中间值刻度文本
 * @param min_val 最小值刻度文本
 */
/**
 * @brief 设置页面B图表刻度标签
 */
void ui_Gas_Measure_Display_set_chart_scale_b(ui_gas_measure_display_t * menu,
                                               uint8_t block_index,
                                               const char * max_val,
                                               const char * mid_val,
                                               const char * min_val) {
    if (menu == NULL || block_index >= 4) return;

    if (max_val != NULL && menu->page_b_blocks[block_index].scale_max_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].scale_max_label, max_val);
    }
    if (mid_val != NULL && menu->page_b_blocks[block_index].scale_mid_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].scale_mid_label, mid_val);
    }
    if (min_val != NULL && menu->page_b_blocks[block_index].scale_min_label != NULL) {
        lv_label_set_text(menu->page_b_blocks[block_index].scale_min_label, min_val);
    }
}
