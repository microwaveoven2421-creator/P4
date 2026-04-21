/**
 * @file ui_menu_manager.c
 * @brief 菜单管理器
 *
 * 实现整个菜单系统的导航、状态管理和所有菜单页面的创建逻辑，
 * 包括主菜单、扩展菜单、设置菜单、烟气测量菜单等全部菜单的
 * 回调函数和交互逻辑。
 */

#include "ui.h"

#define TAG "ui_menu"

#define PRINT_IDX_TYPE          0
#define PRINT_IDX_BT_ADDR       1
#define PRINT_IDX_FW_VER        2
#define PRINT_IDX_VENUE_LINE    1
#define PRINT_IDX_DEVICE_INFO   2

#define SETTING_IDX_LCD_BRIGHTNESS      0
#define SETTING_IDX_BLUETOOTH           1
#define SETTING_IDX_COUNTRY             2
#define SETTING_IDX_LANGUAGE            3
#define SETTING_IDX_SEPARATOR_LED       4
#define SETTING_IDX_HELP_TIP            5
#define SETTING_IDX_FONT                6
#define SETTING_IDX_CURSOR_COLOR        7
#define SETTING_IDX_BOOT_PROTECT        8
#define SETTING_IDX_KEY_SOUND           9

/* 场地管理模拟数据 */
#define MAX_VENUES 50
static char g_venue_list[MAX_VENUES][32];
static uint8_t g_venue_count = 0;
static uint32_t g_auto_venue_id = 1;
static char g_current_new_venue_name[32] = "场地编号.(必需)";
static bool g_is_venue_edited = false;

/* 菜单状态管理 */
menu_state_t current_menu_state = MENU_STATE_MAIN;
static menu_state_t menu_history[8] = {MENU_STATE_MAIN};
static uint8_t history_index = 0;
static lv_timer_t * menu_refresh_timer = NULL;

/* 全局变量定义 */
fuel_type_t g_selected_fuel_type = FUEL_TYPE_SAMPLE_GAS;
uint8_t g_selected_program = 1;

/* 静态变量（各菜单使用） */
/* 注意: 以下指针去掉了 static，以便 ui.c 中的 global_key_event_cb 可以访问 */
ui_temp_diff_menu_t * temp_diff_menu = NULL;
ui_gas_measure_display_t * gas_display_menu = NULL;
static bool gas_program_is_running = false;
ui_test_program_t * test_menu = NULL;
static bool test_program_is_running = false;
static bool co_off_on = false;
static ui_gas_measure_display_t * last_gas_display_menu = NULL;
static ui_test_program_t * last_test_menu = NULL;
ui_pressure_menu_t * pressure_menu = NULL;
ui_value_setting_t * co_limit_menu = NULL;
ui_value_setting_t * fuel_custom_menu = NULL;
static uint8_t g_current_user_fuel_index = 0;
ui_value_setting_t * print_menu = NULL;
static uint8_t print_type_index = 0;
static int32_t saved_venue_line = 0;
static int32_t saved_device_info = 1;
static const char * print_type_options[] = {"HP", "高速打印机", "蓝牙-高速打印机"};
static const uint8_t print_type_count = 3;
ui_value_setting_t * setting_menu = NULL;

static const char * country_options[] = {"China"};
static const uint8_t country_count = 1;
static uint8_t country_index = 0;

static const char * language_options[] = {"中文"};
static const uint8_t language_count = 1;
static uint8_t language_index = 0;

static const char * font_options[] = {"标准"};
static const uint8_t font_count = 1;
static uint8_t font_index = 0;

static const char * cursor_color_options[] = {"经典"};
static const uint8_t cursor_color_count = 1;
static uint8_t cursor_color_index = 0;

/* 函数前向声明 */
static void ui_destroy_fuel_info_custom_menu(void);
static void ui_destroy_co_limit_menu(void);
static void ui_destroy_print_menu(void);
static void ui_rebuild_print_menu(void);
static void ui_destroy_setting_menu(void);
static void ui_rebuild_setting_menu(void);

/* 各菜单的回调函数声明 */
static void action_menu_callback(uint16_t index);

static void main_menu_item_cb(lv_event_t * e);
static void main_menu_bottom_cb(uint8_t index);

static void extend_menu_item_cb(lv_event_t * e);
static void extend_menu_bottom_cb(uint8_t index);

static void storage_menu_item_cb(lv_event_t * e);
static void storage_menu_bottom_cb(uint8_t index);

static void gas_measure_menu_item_cb(lv_event_t * e);
static void gas_measure_menu_bottom_cb(uint8_t index);

static void venue_manage_menu_item_cb(lv_event_t * e);
static void venue_manage_menu_bottom_cb(uint8_t index);

static void new_venue_menu_item_cb(lv_event_t * e);
static void new_venue_menu_bottom_cb(uint8_t index);

static void load_sd_venue_menu_item_cb(lv_event_t * e);
static void load_sd_venue_menu_bottom_cb(uint8_t index);

static void save_sd_venue_menu_item_cb(lv_event_t * e);
static void save_sd_venue_menu_bottom_cb(uint8_t index);

static void view_measure_value_menu_item_cb(lv_event_t * e);
static void view_measure_value_menu_bottom_cb(uint8_t index);

static void del_measure_res_menu_item_cb(lv_event_t * e);
static void del_measure_res_menu_bottom_cb(uint8_t index);

static void data_to_sd_menu_item_cb(lv_event_t * e);
static void data_to_sd_menu_bottom_cb(uint8_t index);

static void memory_info_menu_item_cb(lv_event_t * e);
static void memory_info_menu_bottom_cb(uint8_t index);

static void setting_menu_item_cb(lv_event_t * e);
static void setting_menu_bottom_cb(uint8_t index);

static void device_info_menu_item_cb(lv_event_t * e);
static void device_info_menu_bottom_cb(uint8_t index);

static void fuel_list_menu_item_cb(lv_event_t * e);
static void fuel_list_menu_bottom_cb(uint8_t index);

static void fuel_info_menu_item_cb(lv_event_t * e);
static void fuel_info_menu_bottom_cb(uint8_t index);

static void service_value_menu_item_cb(lv_event_t * e);
static void service_value_menu_bottom_cb(uint8_t index);

static void pressure_menu_item_cb(lv_event_t * e);
static void pressure_menu_bottom_cb(uint8_t index);

static void leak_test_menu_item_cb(lv_event_t * e);
static void leak_test_menu_bottom_cb(uint8_t index);

static void date_time_menu_item_cb(lv_event_t * e);
static void date_time_menu_bottom_cb(uint8_t index);

static void temp_diff_menu_item_cb(lv_event_t * e);
static void temp_diff_menu_bottom_cb(uint8_t index);

static void gas_program_menu_item_cb(lv_event_t * e);
static void gas_program_menu_bottom_cb(uint8_t index);

static void gas_test_program_menu_item_cb(lv_event_t * e);
static void gas_test_program_menu_bottom_cb(uint8_t index);

static void co_limit_menu_item_cb(lv_event_t * e);
static void co_limit_menu_bottom_cb(uint8_t index);

static void fuel_info_custom_menu_item_cb(lv_event_t * e);
static void fuel_info_custom_menu_bottom_cb(uint8_t index);

static void last_menu_item_cb(lv_event_t * e);
static void last_menu_bottom_cb(uint8_t index);

static void print_menu_item_cb(lv_event_t * e);
static void print_menu_bottom_cb(uint8_t index);
/* 菜单配置结构体 */
typedef struct {
    const char * top_hint;
    const char * bottom_left;
    const char * bottom_mid;
    const char * bottom_right;
    bottom_btn_cb_t bottom_cb;
} menu_ui_config_t;

/* 获取当前菜单状态 */
menu_state_t ui_menu_get_current_state(void) {
    return current_menu_state;
}

/* 菜单导航 */
void ui_menu_navigate_to(menu_state_t new_state) {
    if (new_state >= MENU_STATE_MAX) return;

    if (history_index < 7) {
        history_index++;
        menu_history[history_index] = new_state;
    }

    ui_clear_current_menu();

    current_menu_state = new_state;
    g_ui_state.current_menu = new_state;

    switch (new_state) {
        case MENU_STATE_MAIN:
            ui_create_main_menu();
            break;
        case MENU_STATE_EXTEND:
            ui_create_extend_menu();
            break;
        case MENU_STATE_STORAGE:
            ui_create_storage_menu();
            break;
        case MENU_STATE_GAS_MEASURE:
            ui_create_gas_measure_menu();
            break;
        case MENU_STATE_VENUE_MANAGE:
            ui_create_venue_manage_menu();
            break;
        case MENU_STATE_NEW_VENUE:
            ui_create_new_venue_menu();
            break;
        case MENU_STATE_LOAD_SD_VENUE:
            ui_create_load_sd_venue_menu();
            break;
        case MENU_STATE_SAVE_SD_VENUE:
            ui_create_save_sd_venue_menu();
            break;
        case MENU_STATE_VIEW_MEASURE_VALUE:
            ui_create_view_measure_value_menu();
            break;
        case MENU_STATE_DEL_MEASURE_RES:
            ui_create_del_measure_res_menu();
            break;
        case MENU_STATE_MEMORY_INFO:
            ui_create_memory_info_menu();
            break;
        case MENU_STATE_DATA_TO_SD:
            ui_create_data_to_sd_menu();
            break;
        case MENU_STATE_SETTING:
            ui_create_setting_menu();
            break;
        case MENU_STATE_DEVICE_INFO:
            ui_create_device_info_menu();
            break;
        case MENU_STATE_OPTION:
            ui_create_option_menu();
            break;
        case MENU_STATE_MAINTENANCE_RECORD:
            ui_create_maintenance_record_menu();
            break;
        case MENU_STATE_FUEL_LIST:
            ui_create_fuel_list_menu();
            break;
        case MENU_STATE_FUEL_INFO:
            ui_create_fuel_info_menu();
            break;
        case MENU_STATE_SERVICE_VALUE:
            ui_create_service_value_menu();
            break;
        case MENU_STATE_PRESSURE:
            ui_create_pressure_menu();
            break;
        case MENU_STATE_LEAK_TEST:
            ui_create_leak_test_menu();
            break;
        case MENU_STATE_DATE_TIME:
            ui_create_date_time_menu();
            break;
        case MENU_STATE_TEMP_DIFF:
            ui_create_temp_diff_menu();
            break;
        case MENU_STATE_GAS_PROGRAM:
            ui_create_gas_program_menu();
            break;
        case MENU_STATE_GAS_TEST_PROGRAM:
            ui_create_gas_test_program_menu();
            break;
        case MENU_STATE_LAST:
            ui_create_last_menu();
            break;
        case MENU_STATE_CO_LIMIT:
            ui_create_co_limit_menu();
            break;
        case MENU_STATE_FUEL_INFO_CUSTOM:
            ui_create_fuel_info_custom_menu();
            break;
        case MENU_STATE_PRINT:
            ui_create_print_menu();
            break;
        default:
            break;
    }
}

/* 菜单导航 - 返回上一级 */
void ui_menu_go_back(void) {
    if (history_index > 0) {
        history_index--;
        menu_state_t prev_state = menu_history[history_index];

        ui_clear_current_menu();

        current_menu_state = prev_state;
        g_ui_state.current_menu = prev_state;

        switch (prev_state) {
            case MENU_STATE_MAIN:
                ui_create_main_menu();
                break;
            case MENU_STATE_EXTEND:
                ui_create_extend_menu();
                break;
            case MENU_STATE_STORAGE:
                ui_create_storage_menu();
                break;
            case MENU_STATE_GAS_MEASURE:
                ui_create_gas_measure_menu();
                break;
            case MENU_STATE_VENUE_MANAGE:
                ui_create_venue_manage_menu();
                break;
            case MENU_STATE_NEW_VENUE:
                ui_create_new_venue_menu();
                break;
            case MENU_STATE_LOAD_SD_VENUE:
                ui_create_load_sd_venue_menu();
                break;
            case MENU_STATE_SAVE_SD_VENUE:
                ui_create_save_sd_venue_menu();
                break;
            case MENU_STATE_VIEW_MEASURE_VALUE:
                ui_create_view_measure_value_menu();
                break;
            case MENU_STATE_DEL_MEASURE_RES:
                ui_create_del_measure_res_menu();
                break;
            case MENU_STATE_MEMORY_INFO:
                ui_create_memory_info_menu();
                break;
            case MENU_STATE_DATA_TO_SD:
                ui_create_data_to_sd_menu();
                break;
            case MENU_STATE_SETTING:
                ui_create_setting_menu();
                break;
            case MENU_STATE_DEVICE_INFO:
                ui_create_device_info_menu();
                break;
            case MENU_STATE_OPTION:
                ui_create_option_menu();
                break;
            case MENU_STATE_MAINTENANCE_RECORD:
                ui_create_maintenance_record_menu();
                break;
            case MENU_STATE_FUEL_LIST:
                ui_create_fuel_list_menu();
                break;
            case MENU_STATE_FUEL_INFO:
                ui_create_fuel_info_menu();
                break;
            case MENU_STATE_SERVICE_VALUE:
                ui_create_service_value_menu();
                break;
            case MENU_STATE_PRESSURE:
                ui_create_pressure_menu();
                break;
            case MENU_STATE_LEAK_TEST:
                ui_create_leak_test_menu();
                break;
            case MENU_STATE_DATE_TIME:
                ui_create_date_time_menu();
                break;
            case MENU_STATE_TEMP_DIFF:
                ui_create_temp_diff_menu();
                break;
            case MENU_STATE_GAS_PROGRAM:
                ui_create_gas_program_menu();
                break;
            case MENU_STATE_GAS_TEST_PROGRAM:
                ui_create_gas_test_program_menu();
                break;
            case MENU_STATE_LAST:
                ui_create_last_menu();
                break;
            case MENU_STATE_CO_LIMIT:
                ui_create_co_limit_menu();
                break;
            case MENU_STATE_FUEL_INFO_CUSTOM:
                ui_create_fuel_info_custom_menu();
                break;
            case MENU_STATE_PRINT:
                ui_create_print_menu();
                break;
            default:
                ui_create_main_menu();
                break;
        }
    }
}

/* 数据实时刷新 */
static void refresh_service_values_display(void) {
    if (my_menu_cont == NULL) return;

    char buf[32];
    uint16_t count = 16;

    for (uint8_t i = 0; i < count; i++) {
        service_values_get_str(i, buf, sizeof(buf));
        ui_Menu_Text_set(my_menu_cont, i, buf);
    }
}

static void menu_refresh_timer_cb(lv_timer_t * timer) {
    if (my_menu_cont == NULL) return;

    switch (current_menu_state) {
        case MENU_STATE_MAIN:
            break;
        case MENU_STATE_EXTEND:
            break;
        case MENU_STATE_STORAGE:
            break;
        case MENU_STATE_GAS_MEASURE:
            break;
        case MENU_STATE_VENUE_MANAGE:
            break;
        case MENU_STATE_NEW_VENUE:
            break;
        case MENU_STATE_LOAD_SD_VENUE:
            break;
        case MENU_STATE_SAVE_SD_VENUE:
            break;
        case MENU_STATE_VIEW_MEASURE_VALUE:
            break;
        case MENU_STATE_DEL_MEASURE_RES:
            break;
        case MENU_STATE_MEMORY_INFO:
            break;
        case MENU_STATE_DATA_TO_SD:
            break;
        case MENU_STATE_SETTING:
            break;
        case MENU_STATE_DEVICE_INFO:
            break;
        case MENU_STATE_OPTION:
            break;
        case MENU_STATE_MAINTENANCE_RECORD:
            break;
        case MENU_STATE_FUEL_LIST:
            break;
        case MENU_STATE_FUEL_INFO:
            break;
        case MENU_STATE_SERVICE_VALUE:
            service_values_simulate_update();
            refresh_service_values_display();
            break;
        case MENU_STATE_PRESSURE:
            break;
        case MENU_STATE_LEAK_TEST:
            break;
        case MENU_STATE_DATE_TIME:
            break;
        case MENU_STATE_TEMP_DIFF:
            break;
        case MENU_STATE_GAS_PROGRAM:
            break;
        case MENU_STATE_GAS_TEST_PROGRAM:
            break;
        case MENU_STATE_LAST:
            break;
        case MENU_STATE_CO_LIMIT:
            break;
        case MENU_STATE_FUEL_INFO_CUSTOM:
            break;
        case MENU_STATE_PRINT:
            break;
        default:
            break;
    }
}

void ui_menu_start_refresh_timer(uint32_t period_ms) {
    if (menu_refresh_timer != NULL) {
        lv_timer_del(menu_refresh_timer);
        menu_refresh_timer = NULL;
    }

    menu_refresh_timer = lv_timer_create(menu_refresh_timer_cb, period_ms, NULL);
}

void ui_menu_stop_refresh_timer(void) {
    if (menu_refresh_timer != NULL) {
        lv_timer_del(menu_refresh_timer);
        menu_refresh_timer = NULL;
    }
}

/* 清除当前菜单 */
void ui_clear_current_menu(void) {
    ui_menu_stop_refresh_timer();

    if (current_menu_state == MENU_STATE_DATE_TIME) {
        ui_DateTime_Menu_cleanup();
    }

    if (current_menu_state == MENU_STATE_LAST) {
        if (last_gas_display_menu != NULL) {
            ui_Gas_Measure_Display_delete(last_gas_display_menu);
            last_gas_display_menu = NULL;
            my_menu_cont = NULL;
        }
        if (last_test_menu != NULL) {
            ui_Test_Program_delete(last_test_menu);
            last_test_menu = NULL;
            my_menu_cont = NULL;
        }
    }

    if (current_menu_state == MENU_STATE_GAS_PROGRAM) {
        if (gas_display_menu != NULL) {
            ui_Gas_Measure_Display_delete(gas_display_menu);
            gas_display_menu = NULL;
            my_menu_cont = NULL;
        }
    }

    if (current_menu_state == MENU_STATE_GAS_TEST_PROGRAM) {
        if (test_menu != NULL) {
            ui_Test_Program_delete(test_menu);
            test_menu = NULL;
            my_menu_cont = NULL;
        }
    }

    if (current_menu_state == MENU_STATE_TEMP_DIFF) {
        if (temp_diff_menu != NULL) {
            ui_Temp_Diff_Menu_delete(temp_diff_menu);
            temp_diff_menu = NULL;
            my_menu_cont = NULL;
        }
    }

    if (current_menu_state == MENU_STATE_PRESSURE) {
        if (pressure_menu != NULL) {
            ui_Pressure_Menu_delete(pressure_menu);
            pressure_menu = NULL;
            my_menu_cont = NULL;
        }
    }

    if (current_menu_state == MENU_STATE_FUEL_INFO_CUSTOM) {
        ui_destroy_fuel_info_custom_menu();
        my_menu_cont = NULL;
    }

    if (current_menu_state == MENU_STATE_CO_LIMIT) {
        ui_destroy_co_limit_menu();
        my_menu_cont = NULL;
    }

    if (current_menu_state == MENU_STATE_PRINT) {
        ui_destroy_print_menu();
        my_menu_cont = NULL;
    }

    if (current_menu_state == MENU_STATE_SETTING) {
        ui_destroy_setting_menu();
        my_menu_cont = NULL;
    }

    if (my_menu_cont != NULL) {
        if (lv_obj_is_valid(my_menu_cont)) {
            lv_obj_del(my_menu_cont);
        }
        my_menu_cont = NULL;
    }
}

/* 主菜单 */
static void main_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "主菜单项点击: %d", index);

    switch (index) {
        case 0: /* 烟气测量 */
            if(!g_ui_state.is_zero_calibrating)
                ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
            else
                ui_Prompt_Win_enqueue(3000, build_zero_in_progress_prompt_content);
            break;
        case 1: /* 压力测量 */
            if(!g_ui_state.is_zero_calibrating)
                ui_menu_navigate_to(MENU_STATE_PRESSURE);
            else
                ui_Prompt_Win_enqueue(3000, build_zero_in_progress_prompt_content);
            break;
        case 2: /* 差温测量 */
            ui_menu_navigate_to(MENU_STATE_TEMP_DIFF);
            break;
        case 3: /* 上一个测量值 */
            if (last_measurement_is_valid()) {
                ui_menu_navigate_to(MENU_STATE_LAST);
            }
            break;
        case 4: /* 零点校准 */
            ui_zero_calibration_start();
            break;
    }
}

static void main_menu_bottom_cb(uint8_t index) {
    ESP_LOGI(TAG, "主菜单底部按键: %d", index);

    switch (index) {
        case 0: /* 左按键 - 无 */
            break;
        case 1: /* 中间 - 存储 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
        case 2: /* 右按键 - 拓展 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/**
 * @brief 创建主菜单
 */
void ui_create_main_menu(void) {
    const char * names[] = {
        "烟气测量",
        "压力测量",
        "差温测量",
        "设置",
        "零点校准"
    };
    const int16_t heights[] = { 60, 60, 60, 60, 60 };
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        main_menu_item_cb
    );

    /* 上一个测量值 */
    if (!last_measurement_is_valid()) {
        ui_Sub_Menu_set_item_locked(my_menu_cont, 3, true);
    }

    /* 设置顶部提示和底部按键 */
    ui_set_top_hint("测量菜单");
    ui_set_bottom_config(NULL, "存储", "拓展", main_menu_bottom_cb);

    current_menu_state = MENU_STATE_MAIN;
}

/* 拓展菜单 */
static void extend_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "拓展菜单项点击: %d", index);

    switch (index) {
        case 0: /* 设置 */
            ui_menu_navigate_to(MENU_STATE_SETTING);
            break;
        case 1: /* 日期 & 时间 */
            ui_menu_navigate_to(MENU_STATE_DATE_TIME);
            break;
        case 2: /* 服务菜单 */
            if(!g_ui_state.is_zero_calibrating)
                ui_Password_Dialog_create("服务密码", "输入密码", password_result_cb);
            else
                ui_Prompt_Win_enqueue(3000, build_zero_in_progress_prompt_content);
            break;
        case 3: /* 默认设置 */
            show_recovery_dialog();
            break;
        case 4: /* 服务值 */
            ui_menu_navigate_to(MENU_STATE_SERVICE_VALUE);
            break;
        case 5: /* 泄露测试 */
            if(!g_ui_state.is_zero_calibrating)
                ui_menu_navigate_to(MENU_STATE_LEAK_TEST);
            else
                ui_Prompt_Win_enqueue(3000, build_zero_in_progress_prompt_content);
            break;
        case 6: /* 设备信息 */
            ui_menu_navigate_to(MENU_STATE_DEVICE_INFO);
            break;
    }
}

static void extend_menu_bottom_cb(uint8_t index) {
    ESP_LOGI(TAG, "拓展菜单底部按键: %d", index);

    switch (index) {
        case 0: /* 测量 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
        case 1: /* 存储 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
        case 2: /* 设置 */
            ui_menu_navigate_to(MENU_STATE_SETTING);
            break;
    }
}

/**
 * @brief 创建扩展菜单
 */
void ui_create_extend_menu(void) {
    const char * names[] = {
        "设置",
        "日期 & 时间",
        "服务菜单",
        "默认设置",
        "服务值",
        "泄露测试",
        "设备信息"
    };
    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        extend_menu_item_cb
    );

    ui_set_top_hint("扩展菜单");
    ui_set_bottom_config("测量", "存储", "设置", extend_menu_bottom_cb);

    current_menu_state = MENU_STATE_EXTEND;
}

/* 存储菜单 */
static void storage_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "存储菜单项点击: %d", index);

    switch (index) {
        case 0: /* 场地管理 */
            ui_menu_navigate_to(MENU_STATE_VENUE_MANAGE);
            break;
        case 1: /* 删除所有场地 */
            show_del_venue_dialog();
            break;
        case 2: /* 从SD卡导入场地 */
            ui_menu_navigate_to(MENU_STATE_LOAD_SD_VENUE);
            break;
        case 3: /* 保存场地到SD卡 */
            ui_menu_navigate_to(MENU_STATE_SAVE_SD_VENUE);
            break;
        case 4: /* 查看测量值 */
            ui_menu_navigate_to(MENU_STATE_VIEW_MEASURE_VALUE);
            break;
        case 5: /* 删除测量结果 */
            ui_menu_navigate_to(MENU_STATE_DEL_MEASURE_RES);
            break;
        case 6: /* 数据到SD卡 */
            ui_menu_navigate_to(MENU_STATE_DATA_TO_SD);
            break;
        case 7: /* 内存信息 */
            ui_menu_navigate_to(MENU_STATE_MEMORY_INFO);
            break;
    }
}

//"测量", "场地", "拓展"
static void storage_menu_bottom_cb(uint8_t index) {
    ESP_LOGI(TAG, "存储菜单底部按键: %d", index);

    switch (index) {
        case 0: /* 测量 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
        case 1: /* 场地 */
            ui_menu_navigate_to(MENU_STATE_VENUE_MANAGE);
            break;
        case 2: /* 拓展 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

 void ui_create_storage_menu(void) {
    const char * names[] = {
        "场地管理",
        "删除所有场地",
        "从SD卡导入场地",
        "保存场地到SD卡",
        "查看测量值",
        "删除测量结果",
        "数据到SD卡",
        "内存信息"
    };
    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        storage_menu_item_cb
    );

    ui_set_top_hint("存储菜单");
    ui_set_bottom_config("测量", "场地", "拓展", storage_menu_bottom_cb);

    current_menu_state = MENU_STATE_STORAGE;
}

 /* 差温测量 */
static void temp_diff_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* restart  */
            if (temp_diff_menu != NULL && temp_diff_menu->chart != NULL) {
                lv_chart_refresh(temp_diff_menu->chart);
            }
            ui_Temp_Diff_Menu_set_t1(temp_diff_menu, "--.-");
            ui_Temp_Diff_Menu_set_t2(temp_diff_menu, "--.-");
            ui_Temp_Diff_Menu_set_diff(temp_diff_menu, "--.-");
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
    }
}

/**
 * @brief 创建差温测量菜单
 */
void ui_create_temp_diff_menu(void) {
    temp_diff_menu = ui_Temp_Diff_Menu_create(ui_Main_Content);

    if (temp_diff_menu != NULL) {
        my_menu_cont = temp_diff_menu->main_cont;
    }

    ui_Temp_Diff_Menu_set_t1(temp_diff_menu, "--.-");
    ui_Temp_Diff_Menu_set_t2(temp_diff_menu, "--.-");
    ui_Temp_Diff_Menu_set_diff(temp_diff_menu, "--.-");

    ui_Temp_Diff_Menu_set_chart_range(temp_diff_menu, 15, 35);

    ui_set_top_hint("差温测量");
    ui_set_bottom_config("restart", NULL, "返回", temp_diff_menu_bottom_cb);

    current_menu_state = MENU_STATE_TEMP_DIFF;
}

/* 烟气测量 */
static void gas_measure_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "烟气测量菜单项点击: %d", index);

    g_selected_program = index + 1;

    switch (index) {
        case 0: /* 程序一 */
        case 1: /* 程序二 */
        case 2: /* 程序三 */
        case 3: /* 程序四 */
            ui_menu_navigate_to(MENU_STATE_FUEL_LIST);
            break;
        case 4: /* 测试程序 */
            ui_menu_navigate_to(MENU_STATE_GAS_TEST_PROGRAM);
            break;
    }
}

static void gas_measure_menu_bottom_cb(uint8_t index) {
    ESP_LOGI(TAG, "烟气测量菜单底部按键: %d", index);

    switch (index) {
        case 0: /* CO限制 */
            ui_menu_navigate_to(MENU_STATE_CO_LIMIT);
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
    }
}

  void ui_create_gas_measure_menu(void) {
    const char * names[] = {
        "程序一",
        "程序二",
        "程序三",
        "程序四",
        "测试程序"
    };
    const int16_t heights[] = { 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        gas_measure_menu_item_cb
    );

    ui_set_top_hint("选择测量程序");
    ui_set_bottom_config("CO限制", NULL, "返回", gas_measure_menu_bottom_cb);

    current_menu_state = MENU_STATE_GAS_MEASURE;
}

/* 烟气测量->co限制 */

static void co_limit_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 确认 */
            if (co_limit_menu != NULL) {
                ui_Value_Setting_confirm(co_limit_menu);

                /* 索引1是CO限制项（索引0是程序标题） */
                int32_t co_limit = ui_Value_Setting_get_value_int(co_limit_menu, 1);
                ESP_LOGI(TAG, "CO限制设置为: %d ppm", (int)co_limit);

                /* TODO: 保存到配置 */
                // config_set_co_limit(current_program, co_limit);
            }
            break;

        case 1: /* 默认 */
            if (co_limit_menu != NULL) {
                ui_Value_Setting_set_value(co_limit_menu, 1, 8000);
            }
            break;

        case 2: /* 返回 */
            if (co_limit_menu != NULL) {
                if (ui_Value_Setting_is_editing(co_limit_menu)) {
                    ui_Value_Setting_cancel(co_limit_menu);
                } else {
                    ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
                }
            }
            break;
    }
}

/**
 * @brief 创建CO限制菜单
 */
void ui_create_co_limit_menu(void) {
    /* 创建菜单 */
    co_limit_menu = ui_Value_Setting_create(ui_Main_Content, "CO限值", 200);

    if (co_limit_menu == NULL) {
        ESP_LOGE(TAG, "Failed to create CO limit menu");
        return;
    }

    my_menu_cont = co_limit_menu->main_cont;

    /* 添加程序标题项 - 只有左边名字，没有右边数值 */
    char program_title[32];
    snprintf(program_title, sizeof(program_title), "程序%d", g_selected_program);

    value_setting_item_config_t program_config = {
        .title = program_title,
        .unit = NULL,
        .min_value = 0,
        .max_value = 0,
        .step = 0,
        .default_value = 0,
        .decimal_places = 0,
        .readonly = true,
        .show_value = false
    };
    ui_Value_Setting_add_item(co_limit_menu, &program_config);

    /* 添加CO限制设置项 */
    value_setting_item_config_t co_config = {
        .title = "CO限制 [ppm]",
        .unit = NULL,
        .min_value = 0,
        .max_value = 50000,
        .step = 100,
        .default_value = 8000,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(co_limit_menu, &co_config);

    ui_set_top_hint("CO限值");
    ui_set_bottom_config("确认", "默认", "返回", co_limit_menu_bottom_cb);

    current_menu_state = MENU_STATE_CO_LIMIT;
}

static void ui_destroy_co_limit_menu(void) {
    if (co_limit_menu != NULL) {
        /* 先删除LVGL对象树(会清除所有事件回调)，再释放结构体 */
        if (co_limit_menu->main_cont != NULL) {
            lv_obj_del(co_limit_menu->main_cont);
            co_limit_menu->main_cont = NULL;
        }
        my_menu_cont = NULL;
        ui_Value_Setting_cleanup(co_limit_menu);
        co_limit_menu = NULL;
    }
}

 /* 烟气测量->燃料列表 */

 static void fuel_list_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "燃料列表菜单项点击: %d", index);

    if (index < 10) {
        g_selected_fuel_type = (fuel_type_t)index;
        ui_menu_navigate_to(MENU_STATE_FUEL_INFO);
    } else {
        g_selected_fuel_type = (fuel_type_t)index;
        ui_menu_navigate_to(MENU_STATE_FUEL_INFO_CUSTOM);
        ESP_LOGI(TAG, "用户燃料类型 %d", index - 10 + 1);
    }
}

static void fuel_list_menu_bottom_cb(uint8_t index) {

    switch (index) {
        case 0:
            break;
        case 1:
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
            break;
    }
}

/**
 * @brief 创建燃料列表菜单
 */
void ui_create_fuel_list_menu(void) {
    const char * names[] = {
        "Sample gas",
        "天然气",
        "丁烷",
        "丙烷",
        "轻油",
        "重油",
        "干木\xE2\x80\x8B材",
        "煤油",
        "煤",
        "生物柴油",
        "1.用户燃料类型",
        "2.用户燃料类型",
        "3.用户燃料类型",
        "4.用户燃料类型"
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        fuel_list_menu_item_cb
    );

    ui_set_top_hint("燃料类型选择");
    ui_set_bottom_config(NULL, NULL, "返回", fuel_list_menu_bottom_cb);

    current_menu_state = MENU_STATE_FUEL_LIST;
}

/* 烟气测量->燃料列表->用户自定义燃料参数 */
fuel_params_t user_fuel_params[4] = {
    [0] = {
        .type       = FUEL_TYPE_USER_1,
        .name       = "用户燃料1",
        .O2ref      = 3.0f,
        .CO2max     = 12.0f,
        .A2         = 0.60f,
        .B          = 0.000f,
        .Fw         = 0,
        .kWh_factor = 0.0000f,
        .BW_factor  = 1.000f
    },
    [1] = {
        .type       = FUEL_TYPE_USER_2,
        .name       = "用户燃料2",
        .O2ref      = 3.0f,
        .CO2max     = 12.0f,
        .A2         = 0.60f,
        .B          = 0.000f,
        .Fw         = 0,
        .kWh_factor = 0.0000f,
        .BW_factor  = 1.000f
    },
    [2] = {
        .type       = FUEL_TYPE_USER_3,
        .name       = "用户燃料3",
        .O2ref      = 3.0f,
        .CO2max     = 12.0f,
        .A2         = 0.60f,
        .B          = 0.000f,
        .Fw         = 0,
        .kWh_factor = 0.0000f,
        .BW_factor  = 1.000f
    },
    [3] = {
        .type       = FUEL_TYPE_USER_4,
        .name       = "用户燃料4",
        .O2ref      = 3.0f,
        .CO2max     = 12.0f,
        .A2         = 0.60f,
        .B          = 0.000f,
        .Fw         = 0,
        .kWh_factor = 0.0000f,
        .BW_factor  = 1.000f
    }
};

/**
 * @brief 获取用户自定义燃料参数
 */
fuel_params_t* fuel_get_user_params(uint8_t user_index) {
    if (user_index >= 4) return NULL;
    return &user_fuel_params[user_index];
}

/**
 * @brief 设置用户自定义燃料参数
 */
void fuel_set_user_params(uint8_t user_index, const fuel_params_t * params) {
    if (user_index >= 4 || params == NULL) return;
    memcpy(&user_fuel_params[user_index], params, sizeof(fuel_params_t));
}

/* 烟气测量->燃料列表->用户自定义燃料参数 */

static void fuel_info_custom_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 标准 - 恢复为标准值 */
            if (fuel_custom_menu != NULL) {
                /* 索引2-8是数值项 */
                ui_Value_Setting_set_value(fuel_custom_menu, 2, 30);      /* O2ref: 3.0 */
                ui_Value_Setting_set_value(fuel_custom_menu, 3, 120);     /* CO2max: 12.0 */
                ui_Value_Setting_set_value(fuel_custom_menu, 4, 6000);    /* A2: 0.6000 */
                ui_Value_Setting_set_value(fuel_custom_menu, 5, 0);       /* B: 0.0000 */
                ui_Value_Setting_set_value(fuel_custom_menu, 6, 0);       /* Fw: 0 */
                ui_Value_Setting_set_value(fuel_custom_menu, 7, 0);       /* kWh: 0.0000 */
                ui_Value_Setting_set_value(fuel_custom_menu, 8, 1000);    /* BW: 1.000 */

                ESP_LOGI(TAG, "已恢复为标准参数");
            }
            break;

        case 1: /* 选择  */
            if (fuel_custom_menu != NULL) {
                ui_Value_Setting_confirm(fuel_custom_menu);

                fuel_params_t * user_params = fuel_get_user_params(g_current_user_fuel_index);
                if (user_params != NULL) {
                    /* 索引2-8是数值项 */
                    user_params->O2ref = ui_Value_Setting_get_value(fuel_custom_menu, 2);
                    user_params->CO2max = ui_Value_Setting_get_value(fuel_custom_menu, 3);
                    user_params->A2 = ui_Value_Setting_get_value(fuel_custom_menu, 4);
                    user_params->B = ui_Value_Setting_get_value(fuel_custom_menu, 5);
                    user_params->Fw = (uint16_t)ui_Value_Setting_get_value_int(fuel_custom_menu, 6);
                    user_params->kWh_factor = ui_Value_Setting_get_value(fuel_custom_menu, 7);
                    user_params->BW_factor = ui_Value_Setting_get_value(fuel_custom_menu, 8);

                    ESP_LOGI(TAG, "保存用户燃料%d参数: O2ref=%.1f, CO2max=%.1f, A2=%.4f, B=%.4f, Fw=%d, kWh=%.4f, BW=%.3f",
                             g_current_user_fuel_index + 1,
                             user_params->O2ref, user_params->CO2max,
                             user_params->A2, user_params->B,
                             user_params->Fw, user_params->kWh_factor, user_params->BW_factor);
                }

                ui_menu_navigate_to(MENU_STATE_GAS_PROGRAM);

            }
            break;

        case 2: /* 返回 */
            if (fuel_custom_menu != NULL) {
                if (ui_Value_Setting_is_editing(fuel_custom_menu)) {
                    ui_Value_Setting_cancel(fuel_custom_menu);
                } else {
                    ui_menu_navigate_to(MENU_STATE_FUEL_LIST);
                }
            } else {
                ui_menu_navigate_to(MENU_STATE_FUEL_LIST);
            }
            break;
    }
}

/**
 * @brief 创建自定义燃料参数菜单
 */
void ui_create_fuel_info_custom_menu(void) {
    /* 确定当前编辑的是哪个用户燃料 */
    if (g_selected_fuel_type >= FUEL_TYPE_USER_1 && g_selected_fuel_type <= FUEL_TYPE_USER_4) {
        g_current_user_fuel_index = g_selected_fuel_type - FUEL_TYPE_USER_1;
    } else {
        g_current_user_fuel_index = 0;
    }

    /* 获取当前用户燃料参数 */
    fuel_params_t * user_params = fuel_get_user_params(g_current_user_fuel_index);
    if (user_params == NULL) {
        ESP_LOGE(TAG, "无法获取用户燃料参数");
        ui_menu_navigate_to(MENU_STATE_FUEL_LIST);
        return;
    }

    /* 创建标题 */
    static char title_buf[32];
    snprintf(title_buf, sizeof(title_buf), "%d.用户燃料类型", g_current_user_fuel_index + 1);

    /* 创建菜单 */
    fuel_custom_menu = ui_Value_Setting_create(ui_Main_Content, title_buf, 50);

    if (fuel_custom_menu == NULL) {
        ESP_LOGE(TAG, "创建自定义燃料菜单失败");
        return;
    }

    my_menu_cont = fuel_custom_menu->main_cont;

    /* 索引0: 燃料名称（第一行） */
    value_setting_item_config_t name_config = {
        .title = title_buf,
        .unit = NULL,
        .min_value = 0,
        .max_value = 0,
        .step = 0,
        .default_value = 0,
        .decimal_places = 0,
        .readonly = true,
        .show_value = false  /* 右边不显示值 */
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &name_config);

    /* 索引1: 空行（第二行） */
    value_setting_item_config_t empty_config = {
        .title = "",
        .unit = NULL,
        .min_value = 0,
        .max_value = 0,
        .step = 0,
        .default_value = 0,
        .decimal_places = 0,
        .readonly = true,
        .show_value = false
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &empty_config);

    /* 索引2: O2ref [%] - 1位小数 */
    value_setting_item_config_t o2ref_config = {
        .title = "O2ref [%\xE2\x80\x8B]",
        .unit = NULL,
        .min_value = 0,
        .max_value = 210,       /* 21.0 * 10 */
        .step = 1,              /* 0.1 */
        .default_value = (int32_t)(user_params->O2ref * 10),
        .decimal_places = 1,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &o2ref_config);

    /* 索引3: CO2max [%] - 1位小数 */
    value_setting_item_config_t co2max_config = {
        .title = "CO2max [%\xE2\x80\x8B]",
        .unit = NULL,
        .min_value = 0,
        .max_value = 250,       /* 25.0 * 10 */
        .step = 1,              /* 0.1 */
        .default_value = (int32_t)(user_params->CO2max * 10),
        .decimal_places = 1,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &co2max_config);

    /* 索引4: A2 - 4位小数 */
    value_setting_item_config_t a2_config = {
        .title = "A2",
        .unit = NULL,
        .min_value = 0,
        .max_value = 20000,     /* 2.0000 * 10000 */
        .step = 1,              /* 0.0001 */
        .default_value = (int32_t)(user_params->A2 * 10000),
        .decimal_places = 4,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &a2_config);

    /* 索引5: B - 4位小数 */
    value_setting_item_config_t b_config = {
        .title = "B",
        .unit = NULL,
        .min_value = 0,
        .max_value = 10000,     /* 1.0000 * 10000 */
        .step = 1,              /* 0.0001 */
        .default_value = (int32_t)(user_params->B * 10000),
        .decimal_places = 4,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &b_config);

    /* 索引6: Fw - 整数 */
    value_setting_item_config_t fw_config = {
        .title = "Fw",
        .unit = NULL,
        .min_value = 0,
        .max_value = 999,
        .step = 1,
        .default_value = user_params->Fw,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &fw_config);

    /* 索引7: kWh系数 - 4位小数 */
    value_setting_item_config_t kwh_config = {
        .title = "kWh系数",
        .unit = NULL,
        .min_value = 0,
        .max_value = 20000,     /* 2.0000 * 10000 */
        .step = 1,              /* 0.0001 */
        .default_value = (int32_t)(user_params->kWh_factor * 10000),
        .decimal_places = 4,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &kwh_config);

    /* 索引8: BW系数 - 3位小数 */
    value_setting_item_config_t bw_config = {
        .title = "BW系数",
        .unit = NULL,
        .min_value = 0,
        .max_value = 3000,      /* 3.000 * 1000 */
        .step = 1,              /* 0.001 */
        .default_value = (int32_t)(user_params->BW_factor * 1000),
        .decimal_places = 3,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(fuel_custom_menu, &bw_config);

    ui_set_top_hint(title_buf);
    ui_set_bottom_config("标准", "选择", "返回", fuel_info_custom_menu_bottom_cb);

    current_menu_state = MENU_STATE_FUEL_INFO_CUSTOM;
}

static void ui_destroy_fuel_info_custom_menu(void) {
    if (fuel_custom_menu != NULL) {
        if (fuel_custom_menu->main_cont != NULL) {
            lv_obj_del(fuel_custom_menu->main_cont);
            fuel_custom_menu->main_cont = NULL;
        }
        my_menu_cont = NULL;
        ui_Value_Setting_cleanup(fuel_custom_menu);
        fuel_custom_menu = NULL;
    }
}

 /* 烟气测量->燃料列表->燃料参数 */

 static void fuel_info_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void fuel_info_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0:
            break;
        case 1:
            ESP_LOGI(TAG, "确认选择燃料: %d, 程序: %d", g_selected_fuel_type, g_selected_program);
            if (g_selected_program == 5) {
                ui_menu_navigate_to(MENU_STATE_GAS_TEST_PROGRAM);
            } else {
                ui_menu_navigate_to(MENU_STATE_GAS_PROGRAM);
            }
            break;
        case 2:
            ui_menu_navigate_to(MENU_STATE_FUEL_LIST);
            break;
    }
}

/**
 * @brief 创建燃料参数菜单
 */
void ui_create_fuel_info_menu(void) {
    const fuel_params_t * fuel = fuel_get_default_params(g_selected_fuel_type);
    if (fuel == NULL) {
        ESP_LOGE(TAG, "无效的燃料类型: %d", g_selected_fuel_type);
        return;
    }

    static char val_name[32];
    static char val_o2ref[16];
    static char val_co2max[16];
    static char val_a2[16];
    static char val_b[16];
    static char val_fw[16];
    static char val_kwh[16];
    static char val_bw[16];

    snprintf(val_name,   sizeof(val_name),   "%s", fuel->name);
    snprintf(val_o2ref,  sizeof(val_o2ref),  "%.1f", fuel->O2ref);
    snprintf(val_co2max, sizeof(val_co2max), "%.1f", fuel->CO2max);
    snprintf(val_a2,     sizeof(val_a2),     "%.4f", fuel->A2);
    snprintf(val_b,      sizeof(val_b),      "%.4f", fuel->B);
    snprintf(val_fw,     sizeof(val_fw),     "%d", fuel->Fw);
    snprintf(val_kwh,    sizeof(val_kwh),    "%.4f", fuel->kWh_factor);
    snprintf(val_bw,     sizeof(val_bw),     "%.3f", fuel->BW_factor);

    const char * left_texts[] = {
        val_name,
        "",
        "O2ref [%]",
        "CO2max [%]",
        "A2",
        "B",
        "Fw",
        "kWh系数",
        "BW系数"
    };

    const char * right_texts[] = {
        "",
        "",
        val_o2ref,
        val_co2max,
        val_a2,
        val_b,
        val_fw,
        val_kwh,
        val_bw
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60, 60 };
    uint16_t count = sizeof(left_texts) / sizeof(left_texts[0]);

    my_menu_cont = ui_Menu_With_Text_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        left_texts,
        right_texts,
        count,
        fuel_info_menu_item_cb,
        font_normal_32
    );

    ui_set_top_hint("燃料信息");
    ui_set_bottom_config(NULL, "选择", "返回", fuel_info_menu_bottom_cb);

    current_menu_state = MENU_STATE_FUEL_INFO;
}

/* 烟气测量程序一到四 */

static void get_gas_program_title(char * buf, size_t buf_size) {
    const fuel_params_t * fuel = fuel_get_default_params(g_selected_fuel_type);
    const char * fuel_name = (fuel != NULL) ? fuel->name : "未知";

    if (g_selected_program == 5) {
        snprintf(buf, buf_size, "测试程序,%s", fuel_name);
    } else {
        snprintf(buf, buf_size, "程序%d,%s", g_selected_program, fuel_name);
    }
}

static void gas_program_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 开始/停止 */
            if (gas_program_is_running) {

                gas_program_is_running = false;

                if (gas_display_menu != NULL) {
                    last_measurement_save_from_gas_program(gas_display_menu);
                    last_measurement_unlock_menu_item();
                    ESP_LOGI(TAG, "Measurement stopped, data saved");
                }

                /* 更新底部按钮：停止 -> 开始 */
                ui_set_bottom_config("开始", "保存/返回", "保持压力", gas_program_menu_bottom_cb);

                /* 这里可以停止数据采集定时器等 */
                // stop_gas_measurement();

            } else {
                /* 当前已停止 -> 开始 */
                gas_program_is_running = true;

                /* 更新底部按钮：开始 -> 停止 */
                ui_set_bottom_config("停止", "保存/返回", "保持压力", gas_program_menu_bottom_cb);

                /* 这里可以启动数据采集定时器等 */
                // start_gas_measurement();
            }
            break;

        case 1: /* 保存/返回 */
            ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
            break;

        case 2: /* 保持压力 */

            break;
    }
}

/**
 * @brief 创建烟气测量程序菜单
 */
void ui_create_gas_program_menu(void) {
    gas_display_menu = ui_Gas_Measure_Display_create(ui_Main_Content);

    if (gas_display_menu == NULL) {
        ESP_LOGE(TAG, "Failed to create gas display menu");
        return;
    }

    my_menu_cont = gas_display_menu->main_cont;

    /* 初始化页面A - 主界面 */
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 0, "O2", "%", "20.95", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 1, "CO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 2, "NO", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 3, "NO2", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 4, "NOx", "ppm", "0", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 5, "T-烟气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 6, "T-烟气", "°C", "--.-", 0);

    /* 初始化页面B - 上界面 */
    ui_Gas_Measure_Display_set_page_b(gas_display_menu, 0, "T-烟气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_b(gas_display_menu, 1, "T-空气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_b(gas_display_menu, 2, "T-烟气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_b(gas_display_menu, 3, "T-烟气", "°C", "--.-", 0);

    /* 设置图表刻度 */
    ui_Gas_Measure_Display_set_chart_scale_b(gas_display_menu, 0, "1e04", "1e04", "1e04");
    ui_Gas_Measure_Display_set_chart_scale_b(gas_display_menu, 1, "1e04", "1e04", "1e04");
    ui_Gas_Measure_Display_set_chart_scale_b(gas_display_menu, 2, "1e04", "1e04", "1e04");
    ui_Gas_Measure_Display_set_chart_scale_b(gas_display_menu, 3, "1e04", "1e04", "1e04");

    /* 设置图表实际y范围 */
    ui_Gas_Measure_Display_set_chart_range_b(gas_display_menu, 0, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(gas_display_menu, 1, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(gas_display_menu, 2, 0, 500);
    ui_Gas_Measure_Display_set_chart_range_b(gas_display_menu, 3, 0, 500);

    /* 初始化页面C - 下界面 */
    ui_Gas_Measure_Display_set_page_c(gas_display_menu, 0, "T-烟气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_c(gas_display_menu, 1, "T-空气", "°C", "--.-", 0);

    /* 初始化页面D - 左界面 */
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 0, "NOx", "ppm/ref0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 1, "CO", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 2, "NO", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 3, "NOx", "ppm/ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 4, "CO", "mg/Nm³ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 5, "NO", "mg/Nm³ref3.0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_d(gas_display_menu, 6, "NOx", "mg/Nm³ref3.0%O2", "0", 0);

    /* 初始化页面E - 右界面 (带橙色高亮) */
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 0, "T-空气", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 1, "温度差", "°C", "--.-", 0);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 2, "Air ratio", "", "-.--", 0);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 3, "过量空气系数", "%", "---", 0);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 4, "P\xE2\x80\x8Bressure", "hPa", "0.00", 0xFF8C00);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 5, "CO", "ppm/ref0%O2", "0", 0);
    ui_Gas_Measure_Display_set_page_e(gas_display_menu, 6, "NO", "ppm/ref0%O2", "0", 0);

    gas_program_is_running = false;

    static char title_buf[64];

    if (g_selected_fuel_type >= FUEL_TYPE_USER_1 && g_selected_fuel_type <= FUEL_TYPE_USER_4) {
        uint8_t user_index = g_selected_fuel_type - FUEL_TYPE_USER_1;
        snprintf(title_buf, sizeof(title_buf), "程序%d,%d.用户燃料类型",
                 g_selected_program, user_index + 1);
    } else {
        get_gas_program_title(title_buf, sizeof(title_buf));
    }

    ui_set_top_hint(title_buf);
    ui_set_bottom_config("开始", "保存/返回", "保持压力", gas_program_menu_bottom_cb);

    current_menu_state = MENU_STATE_GAS_PROGRAM;
}

/* 烟气测量测试程序 */

static void gas_test_program_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 开始/停止 */
            if (test_program_is_running) {
                /* 当前正在运行 -> 停止 */
                test_program_is_running = false;

                if (test_menu != NULL) {
                    last_measurement_save_from_test_program(test_menu);
                    last_measurement_unlock_menu_item();
                    ESP_LOGI(TAG, "Test program stopped, data saved");
                }

                /* 更新底部按钮：停止 -> 开始 */
                ui_set_bottom_config("开始", "CO off", "保存/返回", gas_test_program_menu_bottom_cb);

            } else {
                /* 当前已停止 -> 开始 */
                test_program_is_running = true;

                /* 更新底部按钮：开始 -> 停止 */
                ui_set_bottom_config("停止", "CO off", "保存/返回", gas_test_program_menu_bottom_cb);
            }
            break;

        case 1: /* CO off */
            if (test_program_is_running) {
                if(co_off_on){
                    co_off_on = false;
                    ui_set_bottom_config("停止", "CO off", "保存/返回", gas_test_program_menu_bottom_cb);
                }else{
                    co_off_on = true;
                    ui_set_bottom_config("停止", "CO on", "保存/返回", gas_test_program_menu_bottom_cb);
                }
            }else{
                if(co_off_on){
                    co_off_on = false;
                    ui_set_bottom_config("开始", "CO off", "保存/返回", gas_test_program_menu_bottom_cb);
                }else{
                    co_off_on = true;
                    ui_set_bottom_config("开始", "CO on", "保存/返回", gas_test_program_menu_bottom_cb);
                }
            }

            break;

        case 2: /* 保存/返回 */
            ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
            break;
    }
}

/**
 * @brief 创建测试程序菜单
 */
void ui_create_gas_test_program_menu(void) {

    test_menu = ui_Test_Program_create(ui_Main_Content);

    if (test_menu != NULL) {
        my_menu_cont = test_menu->main_cont;
    }

    ui_Test_Program_set_block(test_menu, 0, 0, "O2", "%", "20.96", 0);
    ui_Test_Program_set_block(test_menu, 0, 1, "CO", "ppm", "0", 0);
    ui_Test_Program_set_block(test_menu, 0, 2, "NO", "ppm", "0", 0);
    ui_Test_Program_set_block(test_menu, 0, 3, "NO2", "ppm", "-0", 0);
    ui_Test_Program_set_block(test_menu, 0, 4, "T-烟气", "°C", "--.-", 0);
    ui_Test_Program_set_block(test_menu, 0, 5, "T-空气", "°C", "--.-", 0);
    ui_Test_Program_set_block(test_menu, 0, 6, "Pressure", "hPa", "0.00", 0x008000);

    test_program_is_running = false;
    co_off_on = false;

    ui_set_top_hint("测试程序");
    ui_set_bottom_config("开始", "CO off", "保存/返回", gas_test_program_menu_bottom_cb);

    current_menu_state = MENU_STATE_GAS_TEST_PROGRAM;
}

 /* 上一次测量 */

static void last_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
    }
}

/**
 * @brief 创建上一次测量菜单
 */
void ui_create_last_menu(void) {
    if (!last_measurement_is_valid()) {
        ESP_LOGW(TAG, "No valid last measurement data");
        ui_menu_navigate_to(MENU_STATE_MAIN);
        return;
    }

    static char title_buf[64];

    if (g_last_measurement.is_test_program) {
        last_test_menu = ui_Test_Program_create(ui_Main_Content);

        if (last_test_menu != NULL) {
            my_menu_cont = last_test_menu->main_cont;
            last_measurement_restore_to_test_program(last_test_menu);
        }

        snprintf(title_buf, sizeof(title_buf), "测试程序");
    } else {
        last_gas_display_menu = ui_Gas_Measure_Display_create(ui_Main_Content);

        if (last_gas_display_menu != NULL) {
            my_menu_cont = last_gas_display_menu->main_cont;
            last_measurement_restore_to_gas_program(last_gas_display_menu);
        }

        const fuel_params_t * fuel = fuel_get_default_params(g_last_measurement.fuel_type);
        const char * fuel_name = (fuel != NULL) ? fuel->name : "未知";
        snprintf(title_buf, sizeof(title_buf), "程序%d,%s",
                 g_last_measurement.program_number, fuel_name);
    }

    ui_set_top_hint(title_buf);
    ui_set_bottom_config(NULL, NULL, "返回", last_menu_bottom_cb);

    current_menu_state = MENU_STATE_LAST;
}

/* 压力测量 */

static void pressure_menu_bottom_cb(uint8_t index) {

    switch (index) {
        case 0: /* 接受 */
            break;
        case 1: /* 零点 */
            break;
        case 2: /* 保存/返回 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
    }
}

/**
 * @brief 创建压力测量菜单
 */
void ui_create_pressure_menu(void) {
    pressure_menu = ui_Pressure_Menu_create(ui_Main_Content);

    if (pressure_menu != NULL) {
        my_menu_cont = pressure_menu->main_cont;
    }

    ui_Pressure_Menu_set_row(pressure_menu, 0, "差压", "0.0");
    ui_Pressure_Menu_set_row(pressure_menu, 1, "压力2", "0.0");
    ui_Pressure_Menu_set_row(pressure_menu, 2, "压力3", "0.0");
    ui_Pressure_Menu_set_row(pressure_menu, 3, "压力4", "0.0");
    ui_Pressure_Menu_set_peak(pressure_menu, "0.00");
    ui_Pressure_Menu_set_display(pressure_menu, "Pa", "-0.7");

    ui_set_top_hint("压力测量");
    ui_set_bottom_config("接受", "零点", "保存/返回", pressure_menu_bottom_cb);

    current_menu_state = MENU_STATE_PRESSURE;
}

/* 场地管理 */

static void venue_manage_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void venue_manage_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 新建 */
            g_is_venue_edited = false;
            strcpy(g_current_new_venue_name, "场地编号.(必需)");
            ui_menu_navigate_to(MENU_STATE_NEW_VENUE);
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建场地管理菜单
 */
void ui_create_venue_manage_menu(void) {
    const char * names[MAX_VENUES > 0 ? MAX_VENUES : 1];
    int16_t heights[MAX_VENUES > 0 ? MAX_VENUES : 1];
    uint16_t count = 0;

    if (g_venue_count == 0) {
        names[0] = "没有存储的场地名";
        heights[0] = 60;
        count = 1;
    } else {
        for (uint8_t i = 0; i < g_venue_count; i++) {
            names[i] = g_venue_list[i];
            heights[i] = 60;
        }
        count = g_venue_count;
    }

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        100,
        names,
        count,
        venue_manage_menu_item_cb
    );

    ui_set_top_hint("场地管理");
    ui_set_bottom_config("新建", NULL, "返回", venue_manage_menu_bottom_cb);

    current_menu_state = MENU_STATE_VENUE_MANAGE;
}

/* 场地管理->自动编号 */
static void new_venue_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void new_venue_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 保存 */
            if (g_is_venue_edited && g_venue_count < MAX_VENUES) {
                snprintf(g_venue_list[g_venue_count], sizeof(g_venue_list[g_venue_count]), "%s", g_current_new_venue_name);
                g_venue_count++;
                
                ESP_LOGI(TAG, "已保存新场地: %s", g_current_new_venue_name);

                g_is_venue_edited = false;
                strcpy(g_current_new_venue_name, "场地编号.(必需)");
                ui_menu_navigate_to(MENU_STATE_VENUE_MANAGE);
            }
            break;
        case 1: /* 自动编号 */
            snprintf(g_current_new_venue_name, sizeof(g_current_new_venue_name), "场地 %04lu", g_auto_venue_id++);
            g_is_venue_edited = true;
        
            ui_menu_navigate_to(MENU_STATE_NEW_VENUE);
            break;
        case 2: /* 返回 */
            g_is_venue_edited = false;
            strcpy(g_current_new_venue_name, "场地编号.(必需)");
            ui_menu_navigate_to(MENU_STATE_VENUE_MANAGE);
            break;
    }
}

/**
 * @brief 创建新建场地菜单
 */
void ui_create_new_venue_menu(void) {
    const char * names[] = {
        g_current_new_venue_name,  
        "文本(比如名称)",
        "文本(比如名称补充说明)",
        "文本(比如街道和编号)",
        "文本(比如城市和区号)",
        "文本",
        "文本",
        "文本",
        "文本"
    };
    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        count,
        new_venue_menu_item_cb
    );

    for (int i = 1; i <= 8; i++) {
        ui_Sub_Menu_set_item_locked(my_menu_cont, i, true);
    }

    ui_set_top_hint("修改场地");

    if (g_is_venue_edited) {
        ui_set_bottom_config("保存", "自动编号", "返回", new_venue_menu_bottom_cb);
    } else {
        ui_set_bottom_config(NULL, "自动编号", "返回", new_venue_menu_bottom_cb);
    }

    current_menu_state = MENU_STATE_NEW_VENUE;
}

/* 从SD卡导入场地 */
static void load_sd_venue_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void load_sd_venue_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

 void ui_create_load_sd_venue_menu(void) {
    const char * names[] = {
        "从SD卡导入场地",
        "",
        "没有SD卡",
        "",
        "发现场地",
        "",
        "已导入的场地",
        "最大可导入数"
    };

    const int32_t values[] = {
        2147483647,
        2147483647,
        2147483647,
        2147483647,
        0,
        2147483647,
        0,
        32000
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        load_sd_venue_menu_item_cb
    );

    ui_set_top_hint("从SD卡导入场地");
    ui_set_bottom_config(NULL, NULL, "返回", load_sd_venue_menu_bottom_cb);

    current_menu_state = MENU_STATE_LOAD_SD_VENUE;
}

/* 保存场地到SD卡 */

 static void save_sd_venue_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void save_sd_venue_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建保存场地到SD卡菜单
 */
void ui_create_save_sd_venue_menu(void) {
    const char * names[] = {
        "",
        "保存场地到SD卡",
        "",
        "场地",
        "导出"
    };

    const int32_t values[] = {
        2147483647,
        2147483647,
        2147483647,
        g_venue_count,
        0
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        save_sd_venue_menu_item_cb
    );

    ui_set_top_hint("保存场地到SD卡");
    ui_set_bottom_config(NULL, NULL, "返回", save_sd_venue_menu_bottom_cb);

    current_menu_state = MENU_STATE_SAVE_SD_VENUE;
}

/* 查看测量值 */

static void view_measure_value_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void view_measure_value_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* 查看 */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建查看测量值菜单
 */
void ui_create_view_measure_value_menu(void) {
    const char * names[] = {
        "查看测量值",
        "",
        "烟气测量",
        "压力测量"
    };

    const int32_t values[] = {
        2147483647,
        2147483647,
        0,
        0
    };

    const int16_t heights[] = { 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        view_measure_value_menu_item_cb
    );

    ui_set_top_hint("查看测量值");
    ui_set_bottom_config(NULL, "查看", "返回", view_measure_value_menu_bottom_cb);

    current_menu_state = MENU_STATE_VIEW_MEASURE_VALUE;
}

/* 删除测量结果 */

static void del_measure_res_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void del_measure_res_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* 删除 */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建删除测量结果菜单
 */
void ui_create_del_measure_res_menu(void) {
    const char * names[] = {
        "删除测量结果",
        "",
        "所有测量类型",
        "烟气测量",
        "压力测量"
    };

    const int32_t values[] = {
        2147483647,
        2147483647,
        0,
        0,
        0
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        del_measure_res_menu_item_cb
    );

    ui_set_top_hint("删除测量结果");
    ui_set_bottom_config(NULL, "删除", "返回", del_measure_res_menu_bottom_cb);

    current_menu_state = MENU_STATE_DEL_MEASURE_RES;
}

/* 内存信息 */

static void memory_info_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void memory_info_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建内存信息菜单
 */
void ui_create_memory_info_menu(void) {
    const char * names[] = {
        "可用内存[%]",
        "场地",
        "程序1",
        "程序2",
        "程序3",
        "程序4",
        "压力测量"
    };

    const int32_t values[] = {
        100.0,
        g_venue_count,
        0,
        0,
        0,
        0,
        0
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        memory_info_menu_item_cb
    );

    ui_set_top_hint("内存信息");
    ui_set_bottom_config(NULL, NULL, "返回", memory_info_menu_bottom_cb);

    current_menu_state = MENU_STATE_MEMORY_INFO;
}

/* 内存信息 */

static void data_to_sd_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void data_to_sd_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* SD卡? */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/**
 * @brief 创建数据导出到SD卡菜单
 */
void ui_create_data_to_sd_menu(void) {
    const char * names[] = {
        "数据到SD卡",
        "",
        "烟气测量",
        "压力测量"
    };

    const int32_t values[] = {
        2147483647,
        2147483647,
        0,
        0
    };

    const int16_t heights[] = { 60, 60, 60, 60};
    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Menu_With_Value_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        names,
        values,
        count,
        data_to_sd_menu_item_cb
    );

    ui_set_top_hint("数据到SD卡");
    ui_set_bottom_config(NULL, "SD卡?", "返回", data_to_sd_menu_bottom_cb);

    current_menu_state = MENU_STATE_DATA_TO_SD;
}

/* 设置 */

static void setting_bluetooth_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_BLUETOOTH >= menu->item_count) return;

    /* 切换值 */
    system_settings_current.bluetooth = !system_settings_current.bluetooth;
    menu->items[SETTING_IDX_BLUETOOTH].current_value = system_settings_current.bluetooth;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_BLUETOOTH,
                               bluetooth_state_to_str(system_settings_current.bluetooth));
}

static void setting_country_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_COUNTRY >= menu->item_count) return;

    /* 切换到下一个选项 */
    country_index = (country_index + 1) % country_count;
    strncpy(system_settings_current.country, country_options[country_index],
            sizeof(system_settings_current.country) - 1);

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_COUNTRY, country_options[country_index]);
}

static void setting_language_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_LANGUAGE >= menu->item_count) return;

    /* 切换到下一个选项 */
    language_index = (language_index + 1) % language_count;
    strncpy(system_settings_current.language, language_options[language_index],
            sizeof(system_settings_current.language) - 1);

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_LANGUAGE, language_options[language_index]);
}

static void setting_help_tip_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_HELP_TIP >= menu->item_count) return;

    /* 切换值 */
    system_settings_current.help_tip = !system_settings_current.help_tip;
    menu->items[SETTING_IDX_HELP_TIP].current_value = system_settings_current.help_tip;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_HELP_TIP,
                               help_tip_state_to_str(system_settings_current.help_tip));
}

static void setting_font_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_FONT >= menu->item_count) return;

    /* 切换到下一个选项 */
    font_index = (font_index + 1) % font_count;
    system_settings_current.font = (font_type_t)font_index;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_FONT, font_options[font_index]);
}

static void setting_cursor_color_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_CURSOR_COLOR >= menu->item_count) return;

    /* 切换到下一个选项 */
    cursor_color_index = (cursor_color_index + 1) % cursor_color_count;
    system_settings_current.cursor_color = (cursor_color_t)cursor_color_index;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_CURSOR_COLOR, cursor_color_options[cursor_color_index]);
}

static void setting_boot_protect_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_BOOT_PROTECT >= menu->item_count) return;

    /* 切换值 */
    system_settings_current.boot_protect = !system_settings_current.boot_protect;
    menu->items[SETTING_IDX_BOOT_PROTECT].current_value = system_settings_current.boot_protect;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_BOOT_PROTECT,
                               boot_protect_state_to_str(system_settings_current.boot_protect));
}

static void setting_key_sound_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL || SETTING_IDX_KEY_SOUND >= menu->item_count) return;

    /* 切换值 */
    system_settings_current.key_sound = !system_settings_current.key_sound;
    menu->items[SETTING_IDX_KEY_SOUND].current_value = system_settings_current.key_sound;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, SETTING_IDX_KEY_SOUND,
                               key_sound_state_to_str(system_settings_current.key_sound));
}

static void setting_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 打印 */
            /* 先保存当前编辑的值 */
            if (setting_menu != NULL) {
                if (ui_Value_Setting_is_editing(setting_menu)) {
                    ui_Value_Setting_confirm(setting_menu);
                }
                /* 保存亮度值 */
                system_settings_current.lcd_brightness =
                    (uint8_t)ui_Value_Setting_get_value_int(setting_menu, SETTING_IDX_LCD_BRIGHTNESS);
                system_settings_current.separator_led_brightness =
                    (uint8_t)ui_Value_Setting_get_value_int(setting_menu, SETTING_IDX_SEPARATOR_LED);
            }
            ui_menu_navigate_to(MENU_STATE_PRINT);
            break;

        case 1: /* NULL */

            break;

        case 2: /* 测量 - 返回 */
            if (setting_menu != NULL) {
                if (ui_Value_Setting_is_editing(setting_menu)) {
                    ui_Value_Setting_cancel(setting_menu);
                } else {
                    /* 保存当前值 */
                    system_settings_current.lcd_brightness =
                        (uint8_t)ui_Value_Setting_get_value_int(setting_menu, SETTING_IDX_LCD_BRIGHTNESS);
                    system_settings_current.separator_led_brightness =
                        (uint8_t)ui_Value_Setting_get_value_int(setting_menu, SETTING_IDX_SEPARATOR_LED);

                    ui_menu_navigate_to(MENU_STATE_MAIN);
                }
            } else {
                ui_menu_navigate_to(MENU_STATE_MAIN);
            }
            break;
    }
}

static void ui_destroy_setting_menu(void) {
    if (setting_menu != NULL) {
        if (setting_menu->main_cont != NULL) {
            lv_obj_del(setting_menu->main_cont);
            setting_menu->main_cont = NULL;
        }
        my_menu_cont = NULL;
        ui_Value_Setting_cleanup(setting_menu);
        setting_menu = NULL;
    }
}

static void ui_rebuild_setting_menu(void) {
    ui_destroy_setting_menu();

    /* destroy已经删除了my_menu_cont，这里不需要再删 */

    /* 创建新菜单 */
    setting_menu = ui_Value_Setting_create(ui_Main_Content, "设置", 20);
    if (setting_menu == NULL) {
        ESP_LOGE(TAG, "Failed to create setting menu");
        return;
    }

    my_menu_cont = setting_menu->main_cont;

    /* 0: LCD亮度(%) - 可调 0-100 */
    value_setting_item_config_t lcd_config = {
        .title = "LCD亮度(%)",
        .unit = NULL,
        .min_value = 0,
        .max_value = 100,
        .step = 5,
        .default_value = system_settings_current.lcd_brightness,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &lcd_config);

    /* 1: BluetoothLE - 点击切换开/关 */
    value_setting_item_config_t bt_config = {
        .title = "BluetoothLE",
        .unit = NULL,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
        .default_value = system_settings_current.bluetooth,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &bt_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_BLUETOOTH,
                               bluetooth_state_to_str(system_settings_current.bluetooth));
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_BLUETOOTH);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_BLUETOOTH].item_obj,
                        setting_bluetooth_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 2: 国家 - 点击切换选项 */
    value_setting_item_config_t country_config = {
        .title = "国家",
        .unit = NULL,
        .min_value = 0,
        .max_value = country_count - 1,
        .step = 1,
        .default_value = country_index,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &country_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_COUNTRY, system_settings_current.country);
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_COUNTRY);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_COUNTRY].item_obj,
                        setting_country_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 3: 语言 - 点击切换选项 */
    value_setting_item_config_t lang_config = {
        .title = "语言",
        .unit = NULL,
        .min_value = 0,
        .max_value = language_count - 1,
        .step = 1,
        .default_value = language_index,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &lang_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_LANGUAGE, system_settings_current.language);
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_LANGUAGE);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_LANGUAGE].item_obj,
                        setting_language_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 4: 汽水分离器LED亮度 - 可调 0-100 */
    value_setting_item_config_t led_config = {
        .title = "汽水分离器LED亮度(%)",
        .unit = NULL,
        .min_value = 0,
        .max_value = 100,
        .step = 5,
        .default_value = system_settings_current.separator_led_brightness,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &led_config);
    /* 显示带%的值 */
    char led_buf[16];
    snprintf(led_buf, sizeof(led_buf), "%d", system_settings_current.separator_led_brightness);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_SEPARATOR_LED, led_buf);

    /* 5: 帮助提示 - 点击切换开/关 */
    value_setting_item_config_t help_config = {
        .title = "帮助提示",
        .unit = NULL,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
        .default_value = system_settings_current.help_tip,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &help_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_HELP_TIP,
                               help_tip_state_to_str(system_settings_current.help_tip));
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_HELP_TIP);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_HELP_TIP].item_obj,
                        setting_help_tip_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 6: 字体 - 点击切换选项 */
    value_setting_item_config_t font_config = {
        .title = "字体",
        .unit = NULL,
        .min_value = 0,
        .max_value = font_count - 1,
        .step = 1,
        .default_value = font_index,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &font_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_FONT, font_options[font_index]);
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_FONT);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_FONT].item_obj,
                        setting_font_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 7: 光标颜色 - 点击切换选项 */
    value_setting_item_config_t cursor_config = {
        .title = "光标颜色",
        .unit = NULL,
        .min_value = 0,
        .max_value = cursor_color_count - 1,
        .step = 1,
        .default_value = cursor_color_index,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &cursor_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_CURSOR_COLOR, cursor_color_options[cursor_color_index]);
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_CURSOR_COLOR);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_CURSOR_COLOR].item_obj,
                        setting_cursor_color_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 8: 开机保护 - 点击切换开/关 */
    value_setting_item_config_t boot_config = {
        .title = "开机保护",
        .unit = NULL,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
        .default_value = system_settings_current.boot_protect,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &boot_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_BOOT_PROTECT,
                               boot_protect_state_to_str(system_settings_current.boot_protect));
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_BOOT_PROTECT);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_BOOT_PROTECT].item_obj,
                        setting_boot_protect_click_cb, LV_EVENT_CLICKED, setting_menu);

    /* 9: 按键声 - 点击切换开/关 */
    value_setting_item_config_t key_config = {
        .title = "按键声",
        .unit = NULL,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
        .default_value = system_settings_current.key_sound,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(setting_menu, &key_config);
    ui_Value_Setting_set_text(setting_menu, SETTING_IDX_KEY_SOUND,
                               key_sound_state_to_str(system_settings_current.key_sound));
    ui_Value_Setting_remove_default_click(setting_menu, SETTING_IDX_KEY_SOUND);
    lv_obj_add_event_cb(setting_menu->items[SETTING_IDX_KEY_SOUND].item_obj,
                        setting_key_sound_click_cb, LV_EVENT_CLICKED, setting_menu);

    ui_set_top_hint("设置");
    ui_set_bottom_config("打印", NULL , "测量", setting_menu_bottom_cb);

    current_menu_state = MENU_STATE_SETTING;
}

/**
 * @brief 创建系统设置菜单
 */
void ui_create_setting_menu(void) {
    for (uint8_t i = 0; i < country_count; i++) {
        if (strcmp(system_settings_current.country, country_options[i]) == 0) {
            country_index = i;
            break;
        }
    }
    for (uint8_t i = 0; i < language_count; i++) {
        if (strcmp(system_settings_current.language, language_options[i]) == 0) {
            language_index = i;
            break;
        }
    }
    font_index = (uint8_t)system_settings_current.font;
    cursor_color_index = (uint8_t)system_settings_current.cursor_color;

    ui_rebuild_setting_menu();
}

/* 设备信息 */

static void device_info_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "设备信息菜单项点击: %d", index);

    switch (index) {
        case 0: /* MRU RW5 */
            break;
        case 1: /* Firmware version */
            break;
        case 2: /* Subnumber */
            break;
        case 3: /* Meas kernel version */
            break;
        case 4: /* Hardware version */
            break;
        case 5: /* Bootloader version */
            break;
        case 6: /* Serial number */
            break;
        case 7: /* Operating hours */
            break;
        case 8: /* Adjustment data */
            break;
        case 9: /* Pump oper.hours */
            break;
        case 10: /* Pump date */
            break;
        case 11: /* Emi Asia Rel Serie G */
            break;
    }
}

static void device_info_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 维护记录 */
            ui_menu_navigate_to(MENU_STATE_MAINTENANCE_RECORD);
            break;
        case 1: /* 选项 */
            ui_menu_navigate_to(MENU_STATE_OPTION);
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/**
 * @brief 创建设备信息菜单
 */
void ui_create_device_info_menu(void) {
    const char * left_texts[] = {
        "MRU RW5",
        "Firmware version",
        "Subnumber",
        "Meas kernel version",
        "Hardware version",
        "Bootloader version",
        "Serial number",
        "Operating hours",
        "Adjustment data",
        "Pump oper.hours",
        "Pump date",
        "Emi Asia Rel Serie G"
    };

    const char * right_texts[] = {
        "",
        "3.00.11",
        "68",
        "1.04",
        "501",
        "1.11",
        "356562",
        "233.2",
        "06.12.2023",
        "86.72",
        "07.12.2023",
        ""
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(left_texts) / sizeof(left_texts[0]);

    my_menu_cont = ui_Menu_With_Text_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        left_texts,
        right_texts,
        count,
        device_info_menu_item_cb,
        font_char_32
    );

    ui_Sub_Menu_set_item_locked(my_menu_cont, 11 ,true);

    ui_set_top_hint("设备信息");
    ui_set_bottom_config("维护记录", "选项", "返回", device_info_menu_bottom_cb);

    current_menu_state = MENU_STATE_DEVICE_INFO;
}

/* 设备信息->维护记录 */

static void maintenance_record_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void maintenance_record_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_DEVICE_INFO);
            break;
    }
}

/**
 * @brief 创建维护记录菜单
 */
void ui_create_maintenance_record_menu(void) {
    const char * left_texts[] = {
        "Service counter[h]",
        "Service done",
        "Service done",
        "Service done",
        "Service done",
        "Service done",
        "Service done",
        "Service done"
    };

    const char * right_texts[] = {
        "231.6",
        "07.12.2023",
        "----",
        "----",
        "----",
        "----",
        "----",
        "----"
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(left_texts) / sizeof(left_texts[0]);

    my_menu_cont = ui_Menu_With_Text_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        left_texts,
        right_texts,
        count,
        maintenance_record_menu_item_cb,
        font_char_32
    );

    ui_set_top_hint("Service history");
    ui_set_bottom_config(NULL, NULL, "返回", maintenance_record_menu_bottom_cb);

    current_menu_state = MENU_STATE_MAINTENANCE_RECORD;
}

 /* 设备信息->选项 */

static void option_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void option_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 返回 */
            ui_menu_navigate_to(MENU_STATE_DEVICE_INFO);
            break;
    }
}

/**
 * @brief 创建选项菜单
 */
void ui_create_option_menu(void) {
    const char * left_texts[] = {
        "O2 sensor LL",
        "O2 up to 25 percent",
        "CO sensor",
        "NO sensor",
        "NO2 sensor",
        "Draft sensor",
        "CO purge pump",
        "BLE/WLAN-B05",
        "No permanent draft",
        "Due for service active",
        "SD-Card",
        "Diff.temp.measurement",
        "Diff.press.measurement",
        "Instrument leak test"
    };

    const char * right_texts[] = {
        "G007",
        "",
        "G037",
        "G031",
        "G040",
        "P010",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        ""
    };

    const int16_t heights[] = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
    uint16_t count = sizeof(left_texts) / sizeof(left_texts[0]);

    my_menu_cont = ui_Menu_With_Text_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        left_texts,
        right_texts,
        count,
        option_menu_item_cb,
        font_char_32
    );

    ui_Sub_Menu_set_item_locked(my_menu_cont, 11 ,true);

    ui_set_top_hint("Options list");
    ui_set_bottom_config(NULL, NULL, "返回", option_menu_bottom_cb);

    current_menu_state = MENU_STATE_OPTION;
}

 /* 服务值 */

static void service_value_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void service_value_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 气泵 */
            ui_Password_Dialog_create("服务密码", "输入密码", password_result_cb);
            break;
        case 1: /* 清洗泵 */
            ui_Password_Dialog_create("服务密码", "输入密码", password_result_cb);
            break;
        case 2: /* 返回 */
            ui_menu_stop_refresh_timer();
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/**
 * @brief 创建服务值菜单
 */
void ui_create_service_value_menu(void) {
    const char * left_texts[] = {
        "O2 [mV]",
        "CO [mV]",
        "H2 [mV]",
        "NO [mV]",
        "NO2 [mV]",
        "TC-Gas",
        "TC-Air",
        "PT-ref",
        "Pressure 1",
        "V-ref",
        "VDD",
        "T-ADC",
        "U-Batt [V]",
        "I-Batt [A]",
        "T-Batt [V]",
        "U-USB [V]"
    };

    const int16_t heights[] = {
        60, 60, 60, 60,
        60, 60, 60, 60,
        60, 60, 60, 60,
        60, 60, 60, 60
    };

    static char buf[16][32];
    const char * right_texts[16];

    for (int i = 0; i < 16; i++) {
        service_values_get_str(i, buf[i], sizeof(buf[i]));
        right_texts[i] = buf[i];
    }

    uint16_t count = sizeof(left_texts) / sizeof(left_texts[0]);

    my_menu_cont = ui_Menu_With_Text_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        50,
        left_texts,
        right_texts,
        count,
        NULL,
        font_char_32
    );

    ui_set_top_hint("Service values(raw)");
    ui_set_bottom_config("气泵", "清洗泵", "返回", service_value_menu_bottom_cb);

    current_menu_state = MENU_STATE_SERVICE_VALUE;

    ui_menu_start_refresh_timer(2000);
}

 /* 泄露测试 */

static void leak_test_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

}

static void leak_test_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* NULL */
            break;
        case 2: /* 中止 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/**
 * @brief 创建泄露测试菜单
 */
void ui_create_leak_test_menu(void) {
    const char * names[] = {"建立压力\n\n安装测试冒"};
    const int16_t heights[] = {300};

    const bool blink_flags[] = {false};

    uint16_t count = sizeof(names) / sizeof(names[0]);

    my_menu_cont = ui_Blink_Menu_create_with_cb(
        ui_Main_Content,
        heights,
        0,
        200,
        names,
        count,
        leak_test_menu_item_cb,
        blink_flags
    );

    ui_set_top_hint("泄露测试");
    ui_set_bottom_config(NULL, NULL, "中止", leak_test_menu_bottom_cb);

    current_menu_state = MENU_STATE_LEAK_TEST;

}

 /* 日期 & 时间 */

static void date_time_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* 修改 */
            if (ui_DateTime_is_editing()) {
                ui_DateTime_confirm_edit();
            }
            break;
        case 2:
            if (ui_DateTime_is_editing()) {
                ui_DateTime_confirm_edit();
            }
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/**
 * @brief 创建日期时间菜单
 */
void ui_create_date_time_menu(void) {
    my_menu_cont = ui_DateTime_Menu_create(ui_Main_Content, 200);

    ui_set_top_hint("日期 & 时间");
    ui_set_bottom_config(NULL, "修改", "返回", date_time_menu_bottom_cb);

    current_menu_state = MENU_STATE_DATE_TIME;
}

 /* 打印菜单 */
static void print_type_click_cb(lv_event_t * e) {
    LV_UNUSED(e);

    /* 保存当前值 */
    if (print_menu != NULL) {
        uint8_t venue_idx = (print_type_index == 2) ? 3 : 1;
        uint8_t info_idx = (print_type_index == 2) ? 4 : 2;

        if (venue_idx < print_menu->item_count) {
            saved_venue_line = print_menu->items[venue_idx].current_value;
        }
        if (info_idx < print_menu->item_count) {
            saved_device_info = print_menu->items[info_idx].current_value;
        }
    }

    /* 切换类型 */
    print_type_index = (print_type_index + 1) % print_type_count;

    /* 重建菜单 */
    ui_rebuild_print_menu();
}

/* 打印仪器信息点击 - 开关切换 */
static void print_info_click_cb(lv_event_t * e) {
    ui_value_setting_t * menu = (ui_value_setting_t *)lv_event_get_user_data(e);
    if (menu == NULL) return;

    uint8_t info_idx = (print_type_index == 2) ? 4 : 2;
    if (info_idx >= menu->item_count) return;

    /* 切换值 */
    menu->items[info_idx].current_value = !menu->items[info_idx].current_value;
    saved_device_info = menu->items[info_idx].current_value;

    /* 更新显示 */
    ui_Value_Setting_set_text(menu, info_idx, saved_device_info ? "开" : "关");
}

static void print_menu_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* NULL */
            break;
        case 1: /* 测试 */
            ESP_LOGI(TAG, "打印测试 - 类型:%s, 场地行:%d, 仪器信息:%s",
                     print_type_options[print_type_index],
                     (int)saved_venue_line,
                     saved_device_info ? "开" : "关");
            break;
        case 2: /* 返回 */
            if (print_menu != NULL && ui_Value_Setting_is_editing(print_menu)) {
                ui_Value_Setting_cancel(print_menu);
            } else {
                /* 保存最终值 */
                if (print_menu != NULL) {
                    uint8_t venue_idx = (print_type_index == 2) ? 3 : 1;
                    uint8_t info_idx = (print_type_index == 2) ? 4 : 2;
                    if (venue_idx < print_menu->item_count) {
                        saved_venue_line = print_menu->items[venue_idx].current_value;
                    }
                    if (info_idx < print_menu->item_count) {
                        saved_device_info = print_menu->items[info_idx].current_value;
                    }
                }
                ui_menu_navigate_to(MENU_STATE_SETTING);
            }
            break;
    }
}

static void ui_rebuild_print_menu(void) {
    ui_destroy_print_menu();

    /* destroy已经删除了my_menu_cont，这里不需要再删 */

    /* 创建新菜单 */
    print_menu = ui_Value_Setting_create(ui_Main_Content, "打印", 0);
    if (print_menu == NULL) return;

    my_menu_cont = print_menu->main_cont;

    /* 0: 打印机类型 - 点击切换 */
    value_setting_item_config_t type_config = {
        .title = "打印机类型",
        .unit = NULL,
        .min_value = 0,
        .max_value = 2,
        .step = 1,
        .default_value = print_type_index,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(print_menu, &type_config);
    ui_Value_Setting_set_text(print_menu, 0, print_type_options[print_type_index]);
    ui_Value_Setting_remove_default_click(print_menu, 0);
    lv_obj_add_event_cb(print_menu->items[0].item_obj, print_type_click_cb, LV_EVENT_CLICKED, print_menu);

    /* 蓝牙模式额外项 */
    if (print_type_index == 2) {
        /* 1: 蓝牙地址 - 只读无值 */
        value_setting_item_config_t bt_config = {
            .title = "蓝牙地址",
            .unit = NULL,
            .min_value = 0,
            .max_value = 0,
            .step = 0,
            .default_value = 0,
            .decimal_places = 0,
            .readonly = true,
            .show_value = false
        };
        ui_Value_Setting_add_item(print_menu, &bt_config);

        /* 2: Firmware version - 只读显示 */
        value_setting_item_config_t fw_config = {
            .title = "Firmware version",
            .unit = NULL,
            .min_value = 0,
            .max_value = 0,
            .step = 0,
            .default_value = 0,
            .decimal_places = 0,
            .readonly = true,
            .show_value = true
        };
        ui_Value_Setting_add_item(print_menu, &fw_config);
        ui_Value_Setting_set_text(print_menu, 2, "1.3.1.0");
    }

    /* 打印场地行 - 可调0-9 */
    value_setting_item_config_t venue_config = {
        .title = "打印场地行",
        .unit = NULL,
        .min_value = 0,
        .max_value = 9,
        .step = 1,
        .default_value = saved_venue_line,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(print_menu, &venue_config);

    /* 打印仪器信息 - 开关切换 */
    uint8_t info_idx = print_menu->item_count;
    value_setting_item_config_t info_config = {
        .title = "打印仪器信息",
        .unit = NULL,
        .min_value = 0,
        .max_value = 1,
        .step = 1,
        .default_value = saved_device_info,
        .decimal_places = 0,
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(print_menu, &info_config);
    ui_Value_Setting_set_text(print_menu, info_idx, saved_device_info ? "开" : "关");
    ui_Value_Setting_remove_default_click(print_menu, info_idx);
    lv_obj_add_event_cb(print_menu->items[info_idx].item_obj, print_info_click_cb, LV_EVENT_CLICKED, print_menu);

    ui_set_top_hint("打印");
    ui_set_bottom_config(NULL, "测试", "返回", print_menu_bottom_cb);

    current_menu_state = MENU_STATE_PRINT;
}

/**
 * @brief 创建打印菜单
 */
void ui_create_print_menu(void) {
    ui_rebuild_print_menu();
}

static void ui_destroy_print_menu(void) {
    if (print_menu != NULL) {
        if (print_menu->main_cont != NULL) {
            lv_obj_del(print_menu->main_cont);
            print_menu->main_cont = NULL;
        }
        my_menu_cont = NULL;
        ui_Value_Setting_cleanup(print_menu);
        print_menu = NULL;
    }
}


 /* 页面导航 */

/* 导航菜单回调 - 主菜单状态 */
static void nav_main_menu_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    switch (index) {
        case 0: /* 存储 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
        case 1: /* 拓展 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/* 导航菜单回调 - 存储菜单状态 */
static void nav_storage_menu_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    switch (index) {
        case 0: /* 测量 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
        case 1: /* 拓展 */
            ui_menu_navigate_to(MENU_STATE_EXTEND);
            break;
    }
}

/* 导航菜单回调 - 拓展菜单状态 */
static void nav_extend_menu_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    switch (index) {
        case 0: /* 测量 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
        case 1: /* 存储 */
            ui_menu_navigate_to(MENU_STATE_STORAGE);
            break;
    }
}

/* 导航菜单回调 - 返回主菜单 */
static void nav_back_to_main_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_MAIN);
    }
}

/* 导航菜单回调 - 返回存储菜单 */
static void nav_back_to_storage_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_STORAGE);
    }
}

/* 导航菜单回调 - 返回拓展菜单 */
static void nav_back_to_extend_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_EXTEND);
    }
}

/* 导航菜单回调 - 返回烟气测量菜单 */
static void nav_back_to_gas_measure_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
    }
}

/* 导航菜单回调 - 返回设置菜单 */
static void nav_back_to_setting_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_SETTING);
    }
}

/* 导航菜单回调 - 返回设备信息菜单 */
static void nav_back_to_device_info_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_DEVICE_INFO);
    }
}

/* 导航菜单回调 - 返回场地管理菜单 */
static void nav_back_to_venue_manage_callback(uint16_t index) {
    ui_Prompt_Win_Center_close();
    if (index == 0) {
        ui_menu_navigate_to(MENU_STATE_VENUE_MANAGE);
    }
}

/**
 * @brief 显示导航菜单
 * 
 * 根据当前所处的菜单状态，显示合适的导航选项：
 * - 主菜单(MAIN): 可导航到 存储、拓展
 * - 存储菜单(STORAGE): 可导航到 测量、拓展
 * - 拓展菜单(EXTEND): 可导航到 测量、存储
 * - 烟气测量子页面: 返回烟气测量菜单
 * - 存储子页面: 返回存储菜单
 * - 拓展子页面: 返回拓展菜单
 * - 其他深层子页面: 返回对应的父菜单
 */
void show_navigation_menu(void) {
    menu_state_t current = g_ui_state.current_menu;
    ESP_LOGI(TAG,"current menu is: %d", current);
    ESP_LOGI(TAG,"prompt_count is: %d", g_ui_state.prompt_count);
    ESP_LOGI(TAG,"has_confirm_dialog is: %d", g_ui_state.has_confirm_dialog);
    ESP_LOGI(TAG,"has_password_dialog is: %d", g_ui_state.has_password_dialog);

    /* 如果有弹窗正在显示，不处理 */
    if (g_ui_state.prompt_count > 0 || 
        g_ui_state.has_confirm_dialog || 
        g_ui_state.has_password_dialog) {
        return;
    }

    /* 如果居中弹窗已打开，不重复创建 */
    if (ui_Prompt_Win_Center_is_open()) {
        return;
    }

    /* 三个主菜单之间的导航 */
    if (current == MENU_STATE_MAIN) {
        /* 主菜单 -> 存储 / 拓展 */
        static const char * names[] = {"存储", "拓展"};
        static const int16_t heights[] = {60, 60};
        ui_Prompt_Win_Center_create(heights, names, 2, 0, nav_main_menu_callback);
        return;
    }

    if (current == MENU_STATE_STORAGE) {
        /* 存储菜单 -> 测量 / 拓展 */
        static const char * names[] = {"测量", "拓展"};
        static const int16_t heights[] = {60, 60};
        ui_Prompt_Win_Center_create(heights, names, 2, 0, nav_storage_menu_callback);
        return;
    }

    if (current == MENU_STATE_EXTEND) {
        /* 拓展菜单 -> 测量 / 存储 */
        static const char * names[] = {"测量", "存储"};
        static const int16_t heights[] = {60, 60};
        ui_Prompt_Win_Center_create(heights, names, 2, 0, nav_extend_menu_callback);
        return;
    }

    /* 烟气测量相关子页面 -> 返回烟气测量 -> 返回主菜单 */
    if (current == MENU_STATE_GAS_MEASURE ||
        current == MENU_STATE_FUEL_LIST ||
        current == MENU_STATE_FUEL_INFO ||
        current == MENU_STATE_FUEL_INFO_CUSTOM ||
        current == MENU_STATE_GAS_PROGRAM ||
        current == MENU_STATE_GAS_TEST_PROGRAM ||
        current == MENU_STATE_CO_LIMIT) {
        
        if (current == MENU_STATE_GAS_MEASURE) {
            /* 烟气测量菜单本身 -> 返回主菜单 */
            static const char * names[] = {"返回测量菜单"};
            static const int16_t heights[] = {60};
            ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_main_callback);
        } else {
            /* 烟气测量的子页面 -> 返回主菜单 */
            static const char * names[] = {"返回测量菜单"};
            static const int16_t heights[] = {60};
            ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_main_callback);
        }
        return;
    }

    /* 压力测量、差温测量、上一次测量、零点校准 -> 返回主菜单 */
    if (current == MENU_STATE_PRESSURE ||
        current == MENU_STATE_TEMP_DIFF ||
        current == MENU_STATE_LAST ||
        current == MENU_STATE_ZERO_CAL) {
        static const char * names[] = {"返回测量菜单"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_main_callback);
        return;
    }

    /* 存储菜单的子页面 -> 返回存储菜单 */
    if (current == MENU_STATE_VENUE_MANAGE ||
        current == MENU_STATE_DEL_ALL_VENUE ||
        current == MENU_STATE_LOAD_SD_VENUE ||
        current == MENU_STATE_SAVE_SD_VENUE ||
        current == MENU_STATE_VIEW_MEASURE_VALUE ||
        current == MENU_STATE_DEL_MEASURE_RES ||
        current == MENU_STATE_DATA_TO_SD ||
        current == MENU_STATE_MEMORY_INFO) {
        static const char * names[] = {"返回存储菜单"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_storage_callback);
        return;
    }

    /* 场地管理的子页面 -> 返回场地管理 */
    if (current == MENU_STATE_NEW_VENUE) {
        static const char * names[] = {"返回场地管理"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_venue_manage_callback);
        return;
    }

    /* 拓展菜单的直接子页面 -> 返回拓展菜单 */
    if (current == MENU_STATE_SETTING ||
        current == MENU_STATE_DATE_TIME ||
        current == MENU_STATE_SERVICE_MENU ||
        current == MENU_STATE_DEFAULT_SETTING ||
        current == MENU_STATE_SERVICE_VALUE ||
        current == MENU_STATE_LEAK_TEST ||
        current == MENU_STATE_DEVICE_INFO) {
        static const char * names[] = {"返回拓展菜单"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_extend_callback);
        return;
    }

    /* 设置菜单的子页面 -> 返回设置菜单 */
    if (current == MENU_STATE_PRINT) {
        static const char * names[] = {"返回设置菜单"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_setting_callback);
        return;
    }

    /* 设备信息的子页面 -> 返回设备信息 */
    if (current == MENU_STATE_MAINTENANCE_RECORD ||
        current == MENU_STATE_OPTION) {
        static const char * names[] = {"返回设备信息"};
        static const int16_t heights[] = {60};
        ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_device_info_callback);
        return;
    }

    /* 默认情况：返回主菜单 */
    static const char * names[] = {"返回测量菜单"};
    static const int16_t heights[] = {60};
    ui_Prompt_Win_Center_create(heights, names, 1, 0, nav_back_to_main_callback);
}