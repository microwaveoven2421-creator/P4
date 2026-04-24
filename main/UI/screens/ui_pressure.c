#include "screens/ui_pressure.h"
#include "measurement/measurement_record.h"
#include "measurement/measurement_ui.h"
#include "ui_common.h"

#include <stdio.h>

static void snapshot_pressure(char *buf, size_t buf_size, void *user_data)
{
    (void)user_data;
    snprintf(buf, buf_size, "Diff Pressure: 0.0 Pa\nPressure2: 0.0\nPressure3: 0.0\nPressure4: 0.0\nPeak: 0.00");
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

lv_obj_t *ui_pressure_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *host;
    ui_pressure_menu_t *menu;
    const char *title = ui_lang("\xE5\x8E\x8B\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F", "Differential Pressure");

    measurement_create_top_back(screen);
    ui_create_title(screen, title);
    measurement_create_bottom_bar(screen, MEASUREMENT_KIND_PRESSURE, title, snapshot_pressure, NULL);

    host = create_measurement_host(screen);
    menu = ui_Pressure_Menu_create(host);
    if(menu) {
        ui_Pressure_Menu_set_row(menu, 0, "\xE5\xB7\xAE\xE5\x8E\x8B", "0.0");
        ui_Pressure_Menu_set_row(menu, 1, "\xE5\x8E\x8B\xE5\x8A\x9B""2", "0.0");
        ui_Pressure_Menu_set_row(menu, 2, "\xE5\x8E\x8B\xE5\x8A\x9B""3", "0.0");
        ui_Pressure_Menu_set_row(menu, 3, "\xE5\x8E\x8B\xE5\x8A\x9B""4", "0.0");
        ui_Pressure_Menu_set_peak(menu, "0.00");
        ui_Pressure_Menu_set_display(menu, "Pa", "0.0");
    }

    return screen;
}
