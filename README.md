# ESP32-P4 LVGL UI Template

这是一个基于 ESP-IDF 和 LVGL 的 ESP32-P4 多页面触控 UI 工程，当前面向 ESP32-P4 Function EV Board 及 4.3 寸触控屏 BSP。

## 功能概览

- BSP 显示、触摸与 SPIFFS 初始化
- LVGL 9.4 图形界面集成
- 多页面 UI 管理与页面导航
- 首页、设置、燃油、压力、温度、校准、系统信息等业务页面
- 输入框、日期选择器、确认弹窗等公共组件
- 测量页面与记录页面的基础 UI 框架

## 项目结构

```text
.
├── CMakeLists.txt
├── components/
│   ├── bsp_extra/
│   └── esp32_p4_wifi6_touch_lcd_4_3/
├── main/
│   ├── main.c
│   ├── ui_common.c
│   ├── ui_common.h
│   └── UI/
│       ├── ui.c
│       ├── ui_manager.c
│       ├── screens/
│       ├── measurement/
│       ├── comp/
│       ├── Font/
│       └── Image/
├── partitions.csv
├── sdkconfig.defaults
└── README.md
```

## 主要入口

- 应用入口：`main/main.c`
- UI 初始化：`main/UI/ui.c`
- 页面管理：`main/UI/ui_manager.c`
- 公共 UI 工具：`main/ui_common.c`
- 页面实现：`main/UI/screens/*.c`
- 公共组件：`main/UI/comp/*.c`
- 测量模块：`main/UI/measurement/*.c`

## 环境要求

- ESP-IDF 5.x
- ESP32-P4 开发环境
- ESP-IDF Component Manager
- LVGL 9.4.x

依赖在 `main/idf_component.yml` 中声明，包括：

- `lvgl/lvgl`
- `espressif/esp32_p4_function_ev_board`

## 构建与烧录

先进入 ESP-IDF 环境，然后在项目根目录执行：

```bash
idf.py set-target esp32p4
idf.py build
idf.py flash monitor
```

如需修改分区，请更新 `partitions.csv`，并确保 `sdkconfig.defaults` 中的配置与目标硬件一致。

## 新增页面流程

1. 在 `main/UI/screens` 下添加新的 `ui_xxx.c` 和 `ui_xxx.h`。
2. 在 `main/UI/ui_manager.h` 中添加新的 `UI_MENU_xxx` 枚举值。
3. 在 `main/UI/ui_manager.c` 中注册页面创建与导航逻辑。
4. 在 `main/CMakeLists.txt` 的 `SRCS` 中加入新的源文件。
5. 如需复用输入框、日期选择器或确认弹窗，优先使用 `main/UI/comp` 下的公共组件。

## 说明

当前工程保留并编译 `main/UI/**` 下的业务 UI 框架，适合作为 ESP32-P4 + LVGL 触控界面项目的基础模板。
