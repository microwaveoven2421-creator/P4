/**
 * @file ui_manager.h
 *
 * @brief UI管理器
 *
 * @author MO
 * @date 2026-04-21
 *
 * @version 1.0
 *
**/
#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "lvgl.h"

typedef enum {
    UI_MENU_HOME = 0,// 首页
    UI_MENU_SETTING,// 设置
    UI_MENU_FUEL,// 燃油
    UI_MENU_SYSTEM_INFO,// 系统信息
    UI_MENU_CALIBRATION,// 校准
    UI_MENU_MAX// 菜单数量
} ui_menu_t;

void ui_manager_init(void);
void ui_menu_navigate(ui_menu_t menu);
void ui_menu_back(void);

#endif
