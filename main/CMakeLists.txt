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
        "UI/screens/ui_system_info.c"
        "UI/comp/ui_input.c"
        "UI/comp/ui_date_picker.c"

    INCLUDE_DIRS
        "."
        "UI"
        "UI/screens"
        "UI/comp"
)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
)
