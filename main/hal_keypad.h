/**
 * @file hal_keypad.h
 * @brief 物理按键与LVGL输入设备绑定模块
 */

#ifndef _HAL_KEYPAD_H
#define _HAL_KEYPAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/**
 * @brief 初始化物理按键并注册为 LVGL 的 Keypad 输入设备
 * 
 * @note 在调用此函数前，请确保已经调用了 ui_init() 
 *       因为需要使用到全局的 g_keypad_group
 */
void hal_keypad_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_KEYPAD_H */