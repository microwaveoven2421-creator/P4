#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "template_ui.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"

void app_main(void)
{
    bsp_display_start();

    bsp_display_lock(portMAX_DELAY);

    template_ui_init();

    bsp_display_unlock();

    bsp_display_brightness_set(50);// 设置初始亮度为50%

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}