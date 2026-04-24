# esp32_lvgl_template

这是一个基于 ESP32-P4 的 LVGL 多页面 UI 工程，当前保留并编译了 `main/UI/**` 下的业务 UI 框架。

## 当前包含

- BSP 显示初始化
- 背光亮度设置
- LVGL 触摸/显示启动
- 多页面 UI 管理器
- 首页、设置、燃油、压力、温度、校准等页面
- 输入框、日期选择、确认弹窗等公共组件

## 入口

- 应用入口：`main/main.c`
- UI 初始化：`main/UI/ui.c`
- 页面导航：`main/UI/ui_manager.c`
- 公共 UI 工具：`main/ui_common.c`
- 页面实现：`main/UI/screens/*.c`
- 公共组件：`main/UI/comp/*.c`

## 开发建议

1. 新增页面时，在 `main/UI/screens` 下添加 `ui_xxx.c/.h`。
2. 在 `main/UI/ui_manager.h` 添加新的 `UI_MENU_xxx` 枚举。
3. 在 `main/UI/ui_manager.c` 的 `create_menu()` 中注册页面创建函数。
4. 在 `main/CMakeLists.txt` 的 `SRCS` 中加入新的源文件。
