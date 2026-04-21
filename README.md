# esp32_lvgl_template

这是一个精简后的 ESP32-P4 LVGL 模板工程，保留了：

- BSP 显示初始化
- 背光开启
- 触摸随 `bsp_display_start_with_config()` 一起注册
- 一个最小可运行的 LVGL 首页

当前建议从这里开始二次开发：

- 入口文件：[main/main.c](/e:/esp32p4_lvgl_template/main/main.c)
- 模板页面：[main/template_ui.c](/e:/esp32p4_lvgl_template/main/template_ui.c)

已经从 `main` 组件编译链中移除：

- 原业务 UI 框架 `main/UI/**`
- 演示代码 `lv_demos`
- 业务按键绑定 `hal_keypad.c`

如果后续要继续开发，通常只需要：

1. 在 `main/template_ui.c` 里创建你自己的页面和控件
2. 需要多页面时，把 `template_ui.c` 拆成多个 `screen_*.c`
3. 如果要接实体按键，再单独恢复或重写 `hal_keypad.*`
