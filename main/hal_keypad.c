/**
 * @file hal_keypad.c
 * @brief 物理按键与LVGL输入设备绑定实现
 * 
 * 负责读取底层硬件按键状态，并将其映射为 LVGL 的标准控制键值，
 * 最后注册到 LVGL 的输入设备系统中，并绑定到全局焦点组。
 */

#include "hal_keypad.h"
#include "ui.h"        /* 引入 ui.h 以获取 g_keypad_group */

/* 如果你使用的是 ESP32，可以在这里引入 GPIO 驱动头文件 */
// #include "driver/gpio.h"

typedef enum {
    HW_KEY_NONE = 0,
    HW_KEY_OK,
    HW_KEY_ESC,
    HW_KEY_NAV,   /* 导航键 */
    HW_KEY_UP,
    HW_KEY_DOWN,
    HW_KEY_LEFT,
    HW_KEY_RIGHT
} hardware_key_t;

/* 记录上一次按下的键值，用于在按键释放时告诉 LVGL 释放了哪个键 */
static uint32_t last_key = 0;

/**
 * @brief 底层硬件按键读取函数 (需要你根据实际硬件修改)
 * 
 * @return hardware_key_t 返回当前按下的物理按键，如果没有按下则返回 HW_KEY_NONE
 */
static hardware_key_t keypad_get_hardware_key(void) {
    /* 
     * TODO: 在这里添加你的底层 GPIO 读取逻辑
     * 示例 (假设低电平有效):
     * if (gpio_get_level(GPIO_NUM_OK) == 0)    return HW_KEY_OK;
     * if (gpio_get_level(GPIO_NUM_ESC) == 0)   return HW_KEY_ESC;
     * if (gpio_get_level(GPIO_NUM_NAV) == 0)   return HW_KEY_NAV;
     * if (gpio_get_level(GPIO_NUM_UP) == 0)    return HW_KEY_UP;
     * if (gpio_get_level(GPIO_NUM_DOWN) == 0)  return HW_KEY_DOWN;
     * if (gpio_get_level(GPIO_NUM_LEFT) == 0)  return HW_KEY_LEFT;
     * if (gpio_get_level(GPIO_NUM_RIGHT) == 0) return HW_KEY_RIGHT;
     */

    return HW_KEY_NONE; /* 默认没有按键按下 */
}

/**
 * @brief LVGL 输入设备读取回调函数
 * 
 * @param indev 输入设备实例
 * @param data  LVGL 输入数据结构体，用于回传按键状态和键值
 */
static void keypad_read_cb(lv_indev_t * indev, lv_indev_data_t * data) {
    hardware_key_t hw_key = keypad_get_hardware_key();
    uint32_t lv_key = 0;

    /* 将物理按键映射为 LVGL 标准控制键 */
    switch (hw_key) {
        case HW_KEY_OK:     lv_key = LV_KEY_ENTER;  break;
        case HW_KEY_ESC:    lv_key = LV_KEY_ESC;    break;
        case HW_KEY_NAV:    lv_key = LV_KEY_HOME;   break; /* 导航键映射为 HOME */
        case HW_KEY_UP:     lv_key = LV_KEY_UP;     break;
        case HW_KEY_DOWN:   lv_key = LV_KEY_DOWN;   break;
        case HW_KEY_LEFT:   lv_key = LV_KEY_LEFT;   break;
        case HW_KEY_RIGHT:  lv_key = LV_KEY_RIGHT;  break;
        case HW_KEY_NONE:
        default:            lv_key = 0;             break;
    }

    if (lv_key != 0) {
        /* 有按键按下 */
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = lv_key;
        last_key = lv_key; /* 记录当前按下的键，以便释放时使用 */
    } else {
        /* 没有按键按下，或者按键已释放 */
        data->state = LV_INDEV_STATE_RELEASED;
        data->key = last_key; /* 告诉 LVGL 刚刚释放的是哪个键 */
    }
}

/**
 * @brief 初始化物理按键并注册到 LVGL
 */
void hal_keypad_init(void) {
    /* 1. 初始化底层硬件 GPIO (如果还没初始化的话) */
    /* 
     * TODO: gpio_config_t io_conf = {...}; 
     *       gpio_config(&io_conf);
     */

    /* 2. 创建 LVGL 输入设备 (基于 LVGL 9.x API) */
    lv_indev_t * keypad_indev = lv_indev_create();
    
    /* 3. 设置输入设备类型为 KEYPAD */
    lv_indev_set_type(keypad_indev, LV_INDEV_TYPE_KEYPAD);
    
    /* 4. 注册读取回调函数 */
    lv_indev_set_read_cb(keypad_indev, keypad_read_cb);
    
    /* 5. 将该输入设备绑定到全局焦点组 (在 ui_init 中创建的 g_keypad_group) */
    if (g_keypad_group != NULL) {
        lv_indev_set_group(keypad_indev, g_keypad_group);
    } else {
        /* 如果 g_keypad_group 为空，说明 ui_init 还没被调用或者初始化失败 */
    }
}