idf_component_register(
    SRCS
        "main.c"
        "template_ui.c"
        "UI/Font/my_Font_32.c"
    INCLUDE_DIRS
        "."
        "UI")

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
)
