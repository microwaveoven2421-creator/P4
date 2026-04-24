#ifndef MEASUREMENT_UI_H
#define MEASUREMENT_UI_H

#include "lvgl.h"
#include "ui_common.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHART_POINT_COUNT 50

#define font_normal_32 ui_font_32()
#define font_char_32 ui_font_32()
#define font_char_72 ui_font_32()

typedef struct {
    lv_obj_t *main_cont;
    lv_obj_t *page1;
    lv_obj_t *page2;
    lv_obj_t *unit_label;
    lv_obj_t *row_labels[5][2];
    lv_obj_t *peak_left_label;
    lv_obj_t *peak_right_label;
    lv_obj_t *display_unit_label;
    lv_obj_t *display_value_label;
    lv_obj_t *chart;
    lv_chart_series_t *chart_series;
    lv_obj_t *page2_peak_left;
    lv_obj_t *page2_peak_right;
    lv_obj_t *page2_unit_label;
    lv_obj_t *page2_value_label;
    uint8_t current_page;
    uint8_t total_pages;
} ui_pressure_menu_t;

typedef struct {
    lv_obj_t *main_cont;
    lv_obj_t *page1;
    lv_obj_t *chart;
    lv_chart_series_t *chart_series_t1;
    lv_chart_series_t *chart_series_t2;
    lv_obj_t *t2_left_label;
    lv_obj_t *t2_right_label;
    lv_obj_t *t1_left_label;
    lv_obj_t *t1_right_label;
    lv_obj_t *temp_diff_title_label;
    lv_obj_t *temp_diff_value_label;
    lv_obj_t *page2;
    lv_obj_t *page2_t2_unit_label;
    lv_obj_t *page2_t2_pos_label;
    lv_obj_t *page2_t2_icon;
    lv_obj_t *page2_t2_value_label;
    lv_obj_t *page2_t1_unit_label;
    lv_obj_t *page2_t1_pos_label;
    lv_obj_t *page2_t1_icon;
    lv_obj_t *page2_t1_value_label;
    lv_obj_t *page2_diff_unit_label;
    lv_obj_t *page2_diff_pos_label;
    lv_obj_t *page2_diff_value_label;
    uint8_t current_page;
    uint8_t total_pages;
} ui_temp_diff_menu_t;

typedef struct {
    lv_obj_t *name_label;
    lv_obj_t *unit_label;
    lv_obj_t *value_label;
} gas_display_block_ade_t;

typedef struct {
    lv_obj_t *name_label;
    lv_obj_t *unit_label;
    lv_obj_t *value_label;
    lv_obj_t *chart;
    lv_chart_series_t *chart_series;
    lv_obj_t *scale_max_label;
    lv_obj_t *scale_mid_label;
    lv_obj_t *scale_min_label;
} gas_display_block_b_t;

typedef struct {
    lv_obj_t *name_label;
    lv_obj_t *unit_label;
    lv_obj_t *value_label;
} gas_display_block_c_t;

typedef struct {
    lv_obj_t *main_cont;
    lv_obj_t *pages[5];
    gas_display_block_ade_t page_a_blocks[7];
    gas_display_block_b_t page_b_blocks[4];
    gas_display_block_c_t page_c_blocks[2];
    gas_display_block_ade_t page_d_blocks[7];
    gas_display_block_ade_t page_e_blocks[7];
    uint8_t current_page;
} ui_gas_measure_display_t;

ui_pressure_menu_t *ui_Pressure_Menu_create(lv_obj_t *parent);
void ui_Pressure_Menu_set_row(ui_pressure_menu_t *menu, uint8_t row_index,
                              const char *left_text, const char *right_text);
void ui_Pressure_Menu_set_peak(ui_pressure_menu_t *menu, const char *value);
void ui_Pressure_Menu_set_display(ui_pressure_menu_t *menu, const char *unit, const char *value);
void ui_Pressure_Menu_add_chart_point(ui_pressure_menu_t *menu, int32_t value);
void ui_Pressure_Menu_switch_page(ui_pressure_menu_t *menu, uint8_t page);
void ui_Pressure_Menu_delete(ui_pressure_menu_t *menu);
void ui_Pressure_Menu_next_page(ui_pressure_menu_t *menu);
void ui_Pressure_Menu_prev_page(ui_pressure_menu_t *menu);
uint8_t ui_Pressure_Menu_get_current_page(ui_pressure_menu_t *menu);
uint8_t ui_Pressure_Menu_get_total_pages(ui_pressure_menu_t *menu);

ui_temp_diff_menu_t *ui_Temp_Diff_Menu_create(lv_obj_t *parent);
void ui_Temp_Diff_Menu_set_t1(ui_temp_diff_menu_t *menu, const char *value);
void ui_Temp_Diff_Menu_set_t2(ui_temp_diff_menu_t *menu, const char *value);
void ui_Temp_Diff_Menu_set_diff(ui_temp_diff_menu_t *menu, const char *value);
void ui_Temp_Diff_Menu_add_chart_point(ui_temp_diff_menu_t *menu, int32_t t1_value, int32_t t2_value);
void ui_Temp_Diff_Menu_set_chart_range(ui_temp_diff_menu_t *menu, int32_t min, int32_t max);
void ui_Temp_Diff_Menu_switch_page(ui_temp_diff_menu_t *menu, uint8_t page);
void ui_Temp_Diff_Menu_delete(ui_temp_diff_menu_t *menu);
void ui_Temp_Diff_Menu_next_page(ui_temp_diff_menu_t *menu);
void ui_Temp_Diff_Menu_prev_page(ui_temp_diff_menu_t *menu);
uint8_t ui_Temp_Diff_Menu_get_current_page(ui_temp_diff_menu_t *menu);
uint8_t ui_Temp_Diff_Menu_get_total_pages(ui_temp_diff_menu_t *menu);

ui_gas_measure_display_t *ui_Gas_Measure_Display_create(lv_obj_t *parent);
void ui_Gas_Measure_Display_set_page_a(ui_gas_measure_display_t *menu, uint8_t block_index,
                                       const char *name, const char *unit,
                                       const char *value, uint32_t value_color);
void ui_Gas_Measure_Display_set_page_b(ui_gas_measure_display_t *menu, uint8_t block_index,
                                       const char *name, const char *unit,
                                       const char *value, uint32_t value_color);
void ui_Gas_Measure_Display_set_page_c(ui_gas_measure_display_t *menu, uint8_t block_index,
                                       const char *name, const char *unit,
                                       const char *value, uint32_t value_color);
void ui_Gas_Measure_Display_set_page_d(ui_gas_measure_display_t *menu, uint8_t block_index,
                                       const char *name, const char *unit,
                                       const char *value, uint32_t value_color);
void ui_Gas_Measure_Display_set_page_e(ui_gas_measure_display_t *menu, uint8_t block_index,
                                       const char *name, const char *unit,
                                       const char *value, uint32_t value_color);
void ui_Gas_Measure_Display_set_block(ui_gas_measure_display_t *menu, char page,
                                      uint8_t block_index, const char *name,
                                      const char *unit, const char *value,
                                      uint32_t value_color);
void ui_Gas_Measure_Display_set_value(ui_gas_measure_display_t *menu, char page,
                                      uint8_t block_index, const char *value,
                                      uint32_t value_color);
void ui_Gas_Measure_Display_add_chart_point_b(ui_gas_measure_display_t *menu,
                                              uint8_t block_index, int32_t value);
void ui_Gas_Measure_Display_set_chart_range_b(ui_gas_measure_display_t *menu,
                                              uint8_t block_index, int32_t min,
                                              int32_t max);
void ui_Gas_Measure_Display_set_chart_scale_b(ui_gas_measure_display_t *menu,
                                              uint8_t block_index,
                                              const char *max_val,
                                              const char *mid_val,
                                              const char *min_val);
void ui_Gas_Measure_Display_switch_page(ui_gas_measure_display_t *menu, char page);
char ui_Gas_Measure_Display_get_current_page(ui_gas_measure_display_t *menu);
void ui_Gas_Measure_Display_delete(ui_gas_measure_display_t *menu);

#endif
