#include "screens/ui_temperature.h"
#include "measurement/measurement_record.h"
#include "measurement/measurement_ui.h"
#include "ui_common.h"

#include <stdio.h>

static void snapshot_temperature(char *buf, size_t buf_size, void *user_data)
{
    (void)user_data;
    snprintf(buf, buf_size, "T1: --.- C\nT2: --.- C\nDiff: --.- C");
}

static lv_obj_t *create_measurement_host(lv_obj_t *parent)
{
    lv_obj_t *host = lv_obj_create(parent);

    lv_obj_remove_style_all(host);
    lv_obj_set_size(host, 480, 620);
    lv_obj_align(host, LV_ALIGN_TOP_MID, 0, 82);
    lv_obj_clear_flag(host, LV_OBJ_FLAG_SCROLLABLE);

    return host;
}

lv_obj_t *ui_temperature_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *host;
    ui_temp_diff_menu_t *menu;
    const char *title = ui_lang("\xE6\xB8\xA9\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F", "Temperature Difference");

    measurement_create_top_back(screen);
    ui_create_title(screen, title);
    measurement_create_bottom_bar(screen, MEASUREMENT_KIND_TEMP_DIFF, title, snapshot_temperature, NULL);

    host = create_measurement_host(screen);
    menu = ui_Temp_Diff_Menu_create(host);
    if(menu) {
        ui_Temp_Diff_Menu_set_t1(menu, "--.-");
        ui_Temp_Diff_Menu_set_t2(menu, "--.-");
        ui_Temp_Diff_Menu_set_diff(menu, "--.-");
        ui_Temp_Diff_Menu_set_chart_range(menu, 15, 35);
    }

    return screen;
}
