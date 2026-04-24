idf_component_register(
    SRCS
        "main.c"

        "UI/ui.c"
        "UI/ui_manager.c"
        "ui_common.c"
        "UI/Font/my_Font_32.c"

        "UI/screens/ui_home.c"
        "UI/screens/ui_setting.c"
        "UI/screens/ui_fuel.c"
        "UI/screens/ui_fuel_measurement.c"
        "UI/screens/ui_pressure.c"
        "UI/screens/ui_temperature.c"
        "UI/screens/ui_system_info.c"
        "UI/screens/ui_service_record.c"
        "UI/screens/ui_manufacturer_info.c"
        "UI/screens/ui_measurement_unit.c"
        "UI/screens/ui_calibration.c"
        "UI/screens/ui_saved_files.c"
        "UI/measurement/measurement_gas.c"
        "UI/measurement/measurement_temp_diff.c"
        "UI/measurement/measurement_pressure.c"
        "UI/measurement/measurement_record.c"
        "UI/comp/ui_input.c"
        "UI/comp/ui_date_picker.c"
        "UI/comp/ui_confirm.c"

    INCLUDE_DIRS
        "."
        "UI"
        "UI/screens"
        "UI/comp"
        "UI/measurement"
)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB}
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
)
