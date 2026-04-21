/**
 * @file ui.c
 * @brief UI框架核心实现
 *
 * 实现全局变量定义、进度条控制、弹窗队列管理、底部按键处理、
 * 零点校准流程、系统设置管理、对话框逻辑、服务值模拟以及UI初始化。
 */

#include "ui.h"
#include "ui_home.h"
#include "ui_setting.h"
#include "ui_fuel.h"

#include "lvgl.h"

/* 全局变量 */
static ui_lang_t g_lang = LANG_ZH;

static lv_obj_t *g_home = NULL;
static lv_obj_t *g_setting = NULL;
static lv_obj_t *g_fuel = NULL;

// 获取当前语言
ui_lang_t ui_get_language(void)
{
    return g_lang;
}

// 设置当前语言并刷新页面
void ui_set_language(ui_lang_t lang)
{
    g_lang = lang;
    ui_reload();
}

// 刷新页面
void ui_reload(void)
{
    g_home = NULL;
    g_setting = NULL;
    g_fuel = NULL;

    ui_show_home();
}

// 页面切换
void ui_show_home(void)
{
    if (!g_home)
        g_home = ui_home_create();

    lv_screen_load(g_home);
}

// 显示设置页面
void ui_show_setting(void)
{
    if (!g_setting)
        g_setting = ui_setting_create();

    lv_screen_load(g_setting);
}

// 显示燃油页面
void ui_show_fuel(void)
{
    if (!g_fuel)
        g_fuel = ui_fuel_create();

    lv_screen_load(g_fuel);
}

// UI初始化--显示主页
void ui_init(void)
{
    ui_show_home();
}
