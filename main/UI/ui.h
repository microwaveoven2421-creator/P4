/**
 * @file ui.h
 * @brief UI框架主头文件
 *
 * 定义了嵌入式UI系统的全部数据结构、全局变量和函数接口，
 * 包括菜单系统、弹窗、对话框、测量界面等所有UI模块的声明。
 * 基于LVGL 9.x图形库和ESP-IDF平台。
 */
#ifndef UI_H
#define UI_H

void ui_init(void);
void ui_reload(void);

// 页面跳转
void ui_show_home(void);
void ui_show_setting(void);
void ui_show_fuel(void);

// 语言
typedef enum {
    LANG_ZH = 0,
    LANG_EN
} ui_lang_t;

void ui_set_language(ui_lang_t lang);
ui_lang_t ui_get_language(void);

#endif
