#include "screens/ui_fuel_measurement.h"
#include "measurement/measurement_record.h"
#include "measurement/measurement_ui.h"
#include "ui_common.h"

#include <stdio.h>

static void snapshot_gas(char *buf, size_t buf_size, void *user_data)
{
    (void)user_data;
    snprintf(buf, buf_size,
             "O2: 20.95 %%\nCO: 0 ppm\nNO: 0 ppm\nNO2: 0 ppm\nNOx: 0 ppm\nT-Gas: --.- C\nT-Air: --.- C");
}

static const char *g_title_zh = "\xE7\x83\x9F\xE6\xB0\x94\xE6\xB5\x8B\xE9\x87\x8F";
static const char *g_title_en = "Gas Measurement";

void ui_fuel_measurement_set_item(const char *zh, const char *en)
{
    g_title_zh = zh ? zh : "\xE7\x83\x9F\xE6\xB0\x94\xE6\xB5\x8B\xE9\x87\x8F";
    g_title_en = en ? en : "Gas Measurement";
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

static void setup_gas_measurement(ui_gas_measure_display_t *menu)
{
    ui_Gas_Measure_Display_set_page_a(menu, 0, "O2", "%", "20.95", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 1, "CO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 2, "NO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 3, "NO2", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 4, "NOx", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 5, "T-\xE7\x83\x9F\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_a(menu, 6, "T-\xE7\xA9\xBA\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);

    ui_Gas_Measure_Display_set_page_b(menu, 0, "T-\xE7\x83\x9F\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_b(menu, 1, "T-\xE7\xA9\xBA\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_b(menu, 2, "CO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_b(menu, 3, "NO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_chart_scale_b(menu, 0, "500", "250", "0");
    ui_Gas_Measure_Display_set_chart_scale_b(menu, 1, "500", "250", "0");
    ui_Gas_Measure_Display_set_chart_scale_b(menu, 2, "500", "250", "0");
    ui_Gas_Measure_Display_set_chart_scale_b(menu, 3, "500", "250", "0");
    ui_Gas_Measure_Display_set_chart_range_b(menu, 0, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(menu, 1, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(menu, 2, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(menu, 3, 0, 500);

    ui_Gas_Measure_Display_set_page_c(menu, 0, "T-\xE7\x83\x9F\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_c(menu, 1, "T-\xE7\xA9\xBA\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);

    ui_Gas_Measure_Display_set_page_d(menu, 0, "NOx", "ppm/ref0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 1, "CO", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 2, "NO", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 3, "NOx", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 4, "CO", "mg/Nm3ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 5, "NO", "mg/Nm3ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(menu, 6, "NOx", "mg/Nm3ref3.0%O2", "0", 0);

    ui_Gas_Measure_Display_set_page_e(menu, 0, "T-\xE7\xA9\xBA\xE6\xB0\x94", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_e(menu, 1, "\xE6\xB8\xA9\xE5\xBA\xA6\xE5\xB7\xAE", "\xC2\xB0""C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_e(menu, 2, "Air ratio", "", "-.--", 0);
    ui_Gas_Measure_Display_set_page_e(menu, 3, "\xE8\xBF\x87\xE9\x87\x8F\xE7\xA9\xBA\xE6\xB0\x94\xE7\xB3\xBB\xE6\x95\xB0", "%", "---", 0);
    ui_Gas_Measure_Display_set_page_e(menu, 4, "Pressure", "hPa", "0.00", 0xFF8C00);
    ui_Gas_Measure_Display_set_page_e(menu, 5, "CO", "ppm/ref0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_e(menu, 6, "NO", "ppm/ref0%O2", "0", 0);
}

lv_obj_t *ui_fuel_measurement_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *host;
    ui_gas_measure_display_t *menu;
    const char *title = ui_lang(g_title_zh, g_title_en);

    measurement_create_top_back(screen);
    ui_create_title(screen, title);
    measurement_create_bottom_bar(screen, MEASUREMENT_KIND_GAS, title, snapshot_gas, NULL);

    host = create_measurement_host(screen);
    menu = ui_Gas_Measure_Display_create(host);
    if(menu) {
        setup_gas_measurement(menu);
    }

    return screen;
}
