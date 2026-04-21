#include "template_ui.h"

#include "bsp/display.h"
#include "lvgl.h"
#include "string.h"
#include "stdio.h"

LV_FONT_DECLARE(my_Font_32);

typedef struct {
    const char *button_text;
    const char *caption_zh;
    const char *caption_en;
} home_button_item_t;

// 首页按钮数据
static const home_button_item_t home_buttons[] = {
    {"1", "烟气测量", "Gas"},
    {"2", "压力测量", "Pressure"},
    {"3", "温度测量", "Temp"},
    {"4", "设置", "Settings"},
    {"5", "校准", "Calib"},
    {"6", "...", "..."},
};

//烟气测量->燃料列表
static const char *fuel_list_items[][2] = {
    {"天然气", "Natural Gas"},
    {"重油", "Heavy Oil"},
    {"轻油", "Light Oil"},
    {"丙烷", "Propane"},
    {"丁烷", "Butane"},
    {"干木材", "Wood"},
    {"煤", "Coal"},
    {"生物柴油", "Biodiesel"},
    {"煤油", "Kerosene"},
};

//设置->系统设置
static const char *setting_items[][2] = {
    {"日期&时间", "Date & Time"},
    {"LCD亮度(%)", "LCD Brightness"},
    {"蓝牙", "Bluetooth"},
    {"国家/地区", "Region"},
    {"语言", "Language"},
    {"GPS", "GPS"},
    {"WIFI", "WIFI"},
    {"按键音", "Key Sound"},
    {"单位设置", "Unit"},
    {"恢复默认", "Reset"},
    {"系统信息", "Info"},
};
// 设置项的默认值
static char setting_values[16][16] = {
    "",        // 日期&时间
    "50",      // LCD亮度
    "开",      // 蓝牙
    "中国",     // 国家/地区
    "中文",     //语言
    "关",       //GPS
    "关",       //WIFI
    "开",       //按键音
    "公制",     //单位设置
    "",         // 恢复默认
    "",         // 系统信息
};  

//语言ZH-中文 EN-英文
typedef enum {
    LANG_ZH = 0,
    LANG_EN
} language_t;

static language_t g_lang = LANG_ZH;

static lv_obj_t *g_home_screen = NULL;
static lv_obj_t *g_fuel_list_screen = NULL;
static lv_obj_t *g_setting_screen = NULL;
static lv_obj_t *setting_value_labels[16];

static void show_home_screen(void);
static void show_fuel_list_screen(void);
static void show_setting_screen(void);
//统一语言函数
static const char* lang_text(const char* zh, const char* en)
{
    return (g_lang == LANG_ZH) ? zh : en;
}

void ui_reload_all(void)
{
    g_home_screen = NULL;
    g_setting_screen = NULL;
    g_fuel_list_screen = NULL;

    show_home_screen();
}

static void fuel_list_back_event_cb(lv_event_t *e)// 燃料列表页面的cb
{
    LV_UNUSED(e);
    show_home_screen();
}

static void setting_back_event_cb(lv_event_t *e)// 设置页面的cb
{
    LV_UNUSED(e);
    show_home_screen();
}

static void setting_item_event_cb(lv_event_t *e)
{
    uint32_t index =
        (uint32_t)(uintptr_t)
        lv_event_get_user_data(e);

    switch(index)
    {
        case 0: //日期&时间
        {
            // 这里可以添加日期时间设置的逻辑，例如弹出一个日期时间选择器
            break;
        }

        case 1: //亮度切换
        {
            static int brightness = 50;

            brightness += 10;

            if(brightness > 100)
                brightness = 10;

            sprintf(setting_values[1],
                    "%d",
                    brightness);

            lv_label_set_text(
                setting_value_labels[1],
                setting_values[1]);

            // 真调背光（可打开）
             bsp_display_brightness_set(brightness);

            break;
        }

        case 2: //蓝牙
        {
            if(strcmp(setting_values[2],"开")==0)
                strcpy(setting_values[2],"关");
            else
                strcpy(setting_values[2],"开");

            lv_label_set_text(
                setting_value_labels[2],
                setting_values[2]);

            break;
        }

        case 4: // 语言
        {
            if (g_lang == LANG_ZH)
            {
                g_lang = LANG_EN;
                strcpy(setting_values[4], "EN");
            }
            else
            {
                g_lang = LANG_ZH;
                strcpy(setting_values[4], "中文");
            }

            lv_label_set_text(
            setting_value_labels[4],
            setting_values[4]);
            ui_reload_all();
            show_setting_screen();  // 重新加载当前页面

            break;
        }

        case 5: //GPS
        {
            if(strcmp(setting_values[5],"开")==0)
                strcpy(setting_values[5],"关");
            else
                strcpy(setting_values[5],"开");

            lv_label_set_text(
                setting_value_labels[5],
                setting_values[5]);

            break;
        
        }
        case 6: //WIFI
        {
            if(strcmp(setting_values[6],"开")==0)
                strcpy(setting_values[6],"关");
            else
                strcpy(setting_values[6],"开");

            lv_label_set_text(
                setting_value_labels[6],
                setting_values[6]);

            break;
        }

        case 7: //按键音
        {
            if(strcmp(setting_values[index],"开")==0)
                strcpy(setting_values[index],"关");
            else
                strcpy(setting_values[index],"开");

            lv_label_set_text(
                setting_value_labels[index],
                setting_values[index]);

            break;
        }
    }
}

static void home_button_event_cb(lv_event_t *e)// 首页按钮的cb
{
    uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    if (index == 0) {
        show_fuel_list_screen();// 烟气测量
    }

    if (index == 3) {
    show_setting_screen();     // 设置
}
}

// 创建顶部LOGO区域 
static void create_logo_area(lv_obj_t *parent)
{
    lv_obj_t *logo_area = lv_obj_create(parent);
    lv_obj_remove_style_all(logo_area);
    lv_obj_set_size(logo_area, 480, 280);
    lv_obj_align(logo_area, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(logo_area, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *logo = lv_label_create(logo_area);
    lv_label_set_text(logo, "LOGO");// 这里可以替换为实际的LOGO图片，使用函数lv_img_create()，并设置图片源
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(logo, lv_color_hex(0x0F172A), 0);// 这里设置LOGO的文字颜色，实际使用图片时可以忽略
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *subtitle = lv_label_create(logo_area);
    lv_label_set_text(subtitle, 
        lang_text("主页", "Home"));
    lv_obj_set_style_text_font(subtitle, &my_Font_32, 0);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x475569), 0);
    lv_obj_align_to(subtitle, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
}

static lv_obj_t *create_menu_button(lv_obj_t *parent, const home_button_item_t *item, uint32_t index)
{
    lv_obj_t *item_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(item_cont);
    lv_obj_set_size(item_cont, 140, 160);
    lv_obj_set_style_pad_all(item_cont, 0, 0);
    lv_obj_set_style_pad_gap(item_cont, 8, 0);
    lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(item_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *button = lv_button_create(item_cont);
    lv_obj_set_size(button, 110, 110);
    lv_obj_set_style_radius(button, 22, 0);
    lv_obj_set_style_border_width(button, 2, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x2563EB), LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(button, 0, 0);
    lv_obj_set_style_shadow_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(button, home_button_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)index);

    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, item->button_text);
    lv_obj_set_style_text_font(button_label, &lv_font_montserrat_34, 0);
    lv_obj_set_style_text_color(button_label, lv_color_hex(0x1D4ED8), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(button_label, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED);
    lv_obj_center(button_label);

    lv_obj_t *caption = lv_label_create(item_cont);
    lv_label_set_text(caption, 
        lang_text(item->caption_zh, item->caption_en));// 设置按钮下方的文字，根据语言选择显示中文或英文
    lv_label_set_long_mode(caption, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(caption, 140);
    lv_obj_set_height(caption, 36);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(caption, &my_Font_32, 0);
    lv_obj_set_style_text_letter_space(caption, -2, 0);
    lv_obj_set_style_text_color(caption, lv_color_hex(0x334155), 0);

    return item_cont;
}

static void create_button_grid(lv_obj_t *parent)
{
    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 460, 400);
    lv_obj_align(grid, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_pad_row(grid, 20, 0);
    lv_obj_set_style_pad_column(grid, 10, 0);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    static int32_t col_dsc[] = {140, 140, 140, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {180, 180, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    for (uint32_t i = 0; i < (sizeof(home_buttons) / sizeof(home_buttons[0])); i++) {
        uint32_t row = i / 3;
        uint32_t col = i % 3;
        lv_obj_t *item = create_menu_button(grid, &home_buttons[i], i);
        lv_obj_set_grid_cell(item, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);
    }
}

// 创建燃料列表页面
static void create_fuel_list_page(lv_obj_t *screen)
{
    lv_obj_t *title_bar = lv_obj_create(screen);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, 480, 120);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0xE2E8F0), 0);

    lv_obj_t *title = lv_label_create(title_bar);
    lv_label_set_text(title, 
                        lang_text("燃料列表", "Fuel List"));
    lv_obj_set_style_text_font(title, &my_Font_32, 0);
    lv_obj_set_style_text_letter_space(title, 6, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x0F172A), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *back_btn = lv_button_create(screen);
    lv_obj_set_size(back_btn, 90, 44);// 这里设置返回按钮的大小
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 20, 24);
    lv_obj_set_style_radius(back_btn, 12, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_add_event_cb(back_btn, fuel_list_back_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, 
                        lang_text("返回", "Back"));
    lv_obj_set_style_text_font(back_label, &my_Font_32, 0);
    lv_obj_set_style_text_letter_space(back_label, -2, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x1E293B), 0);
    lv_obj_center(back_label);

    // 创建列表容器
    lv_obj_t *list_cont = lv_obj_create(screen);
    lv_obj_remove_style_all(list_cont);
    lv_obj_set_size(list_cont, 440, 650);// 设置列表容器的大小
    lv_obj_align(list_cont, LV_ALIGN_BOTTOM_MID, 0, -8);// 设置列表容器的位置，底部8像素的间距
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(list_cont, 0, 0);// 设置列表容器的内边距
    lv_obj_set_style_pad_row(list_cont, 10, 0);// 设置列表项之间的垂直间距
    lv_obj_set_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN);// 设置列表容器为垂直布局
    lv_obj_set_flex_align(list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);// 设置列表项在容器中的对齐方式
    lv_obj_set_scroll_dir(list_cont, LV_DIR_VER);// 设置列表容器只能垂直滚动
    lv_obj_set_scrollbar_mode(list_cont, LV_SCROLLBAR_MODE_OFF);

    for (uint32_t i = 0; i < (sizeof(fuel_list_items) / sizeof(fuel_list_items[0])); i++) {
        lv_obj_t *item = lv_button_create(list_cont);
        lv_obj_set_size(item, 440, 60);// 设置列表项的大小
        lv_obj_set_style_radius(item, 10, 0);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xF8FAFC), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xDBEAFE), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0xCBD5E1), 0);
        lv_obj_set_style_shadow_width(item, 0, 0);

        lv_obj_t *label = lv_label_create(item);
        lv_label_set_text(label, 
                            lang_text(fuel_list_items[i][0], fuel_list_items[i][1]));
        lv_obj_set_style_text_font(label, &my_Font_32, 0);
        lv_obj_set_style_text_letter_space(label, -2, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x334155), 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 18, 0);
    }
}

// 创建设置页面
static void create_setting_page(lv_obj_t *screen)
{
    lv_obj_t *title_bar = lv_obj_create(screen);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, 480, 120);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0xE2E8F0), 0);

    lv_obj_t *title = lv_label_create(title_bar);
    lv_label_set_text(title, 
                        lang_text("设置", "Settings"));
    lv_obj_set_style_text_font(title, &my_Font_32, 0);
    lv_obj_set_style_text_letter_space(title, 6, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x0F172A), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *back_btn = lv_button_create(screen);
    lv_obj_set_size(back_btn, 90, 44);// 这里设置返回按钮的大小
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 20, 24);
    lv_obj_set_style_radius(back_btn, 12, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_add_event_cb(back_btn, fuel_list_back_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, 
                        lang_text("返回", "Back"));
    lv_obj_set_style_text_font(back_label, &my_Font_32, 0);
    lv_obj_set_style_text_letter_space(back_label, -2, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x1E293B), 0);
    lv_obj_center(back_label);

    // 创建列表容器
    lv_obj_t *list_cont = lv_obj_create(screen);
    lv_obj_remove_style_all(list_cont);
    lv_obj_set_size(list_cont, 440, 650);// 设置列表容器的大小
    lv_obj_align(list_cont, LV_ALIGN_BOTTOM_MID, 0, -8);// 设置列表容器的位置，底部8像素的间距
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(list_cont, 0, 0);// 设置列表容器的内边距
    lv_obj_set_style_pad_row(list_cont, 10, 0);// 设置列表项之间的垂直间距
    lv_obj_set_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN);// 设置列表容器为垂直布局
    lv_obj_set_flex_align(list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);// 设置列表项在容器中的对齐方式
    lv_obj_set_scroll_dir(list_cont, LV_DIR_VER);// 设置列表容器只能垂直滚动
    lv_obj_set_scrollbar_mode(list_cont, LV_SCROLLBAR_MODE_OFF);

    for (uint32_t i = 0; i < (sizeof(setting_items) / sizeof(setting_items[0])); i++) {
        lv_obj_t *item = lv_button_create(list_cont);
        lv_obj_set_size(item, 440, 60);// 设置列表项的大小
        lv_obj_set_style_radius(item, 10, 0);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xF8FAFC), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xDBEAFE), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0xCBD5E1), 0);
        lv_obj_set_style_shadow_width(item, 0, 0);

        lv_obj_add_event_cb(item,setting_item_event_cb,LV_EVENT_CLICKED,(void *)(uintptr_t)i);
        //左边的设置项名称
        lv_obj_t *label = lv_label_create(item);
        lv_label_set_text(label,
                            lang_text(setting_items[i][0], setting_items[i][1]));
        lv_obj_set_style_text_font(label, &my_Font_32, 0);
        lv_obj_set_style_text_letter_space(label, -2, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x334155), 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 18, 0);

        // 右边的设置项数值
setting_value_labels[i] = lv_label_create(item);

lv_label_set_text(
    setting_value_labels[i],
    setting_values[i]);

lv_obj_set_style_text_font(
    setting_value_labels[i],
    &my_Font_32,
    0);

lv_obj_set_style_text_letter_space(
    setting_value_labels[i],
    -2,
    0);

lv_obj_set_style_text_color(
    setting_value_labels[i],
    lv_color_hex(0x1D4ED8),
    0);

lv_obj_align(
    setting_value_labels[i],
    LV_ALIGN_RIGHT_MID,
    -18,
    0);
    }
}

//主屏幕
static void show_home_screen(void)
{
    if (g_home_screen == NULL) {
        g_home_screen = lv_obj_create(NULL);
        lv_obj_remove_style_all(g_home_screen);
        lv_obj_set_style_bg_opa(g_home_screen, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(g_home_screen, lv_color_hex(0xFFFFFF), 0);
        lv_obj_clear_flag(g_home_screen, LV_OBJ_FLAG_SCROLLABLE);

        create_logo_area(g_home_screen);
        create_button_grid(g_home_screen);
    }

    lv_screen_load(g_home_screen);
}

// 燃料列表页面
static void show_fuel_list_screen(void)
{
    if (g_fuel_list_screen == NULL) {
        g_fuel_list_screen = lv_obj_create(NULL);
        lv_obj_remove_style_all(g_fuel_list_screen);
        lv_obj_set_style_bg_opa(g_fuel_list_screen, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(g_fuel_list_screen, lv_color_hex(0xFFFFFF), 0);
        lv_obj_clear_flag(g_fuel_list_screen, LV_OBJ_FLAG_SCROLLABLE);

        create_fuel_list_page(g_fuel_list_screen);
    }

    lv_screen_load(g_fuel_list_screen);
}

// 设置页面
static void show_setting_screen(void)
{
    if(g_setting_screen==NULL)
    {
        g_setting_screen=
            lv_obj_create(NULL);

        lv_obj_remove_style_all(
            g_setting_screen);

        lv_obj_set_style_bg_opa(
            g_setting_screen,
            LV_OPA_COVER,
            0);

        lv_obj_set_style_bg_color(
            g_setting_screen,
            lv_color_hex(0xFFFFFF),
            0);

        create_setting_page(
            g_setting_screen);
    }

    lv_screen_load(g_setting_screen);
}
void template_ui_init(void)
{
    show_home_screen();
}
