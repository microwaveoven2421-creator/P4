# 嵌入式UI框架 API 文档

> 基于 LVGL 9.x + ESP-IDF 的烟气分析仪 UI 系统  
> 屏幕分辨率: 480×800 | 输入方式: 物理按键 + 触摸屏

---

## 1. 系统架构概述

### 1.1 文件结构

| 文件 | 职责 |
|------|------|
| `ui.h` | 所有数据结构、全局变量、函数声明 |
| `ui.c` | 全局变量定义、按键处理、弹窗队列、零点校准、系统设置、UI初始化 |
| `ui_menu_manager.c` | 菜单导航状态机、所有菜单页面的创建和回调逻辑 |
| `ui_Screen_Main.c` | 主屏幕基础UI元素（背景、进度条、内容容器、顶部装饰杆） |
| `ui_Top_Bottom_Text.c` | 顶部提示文字 + 底部三个功能按键区域 |
| `ui_Sub_Menu.c` | 基础子菜单组件（灰色背景 + 渐变分隔线） |
| `ui_Menu_With_Text.c` | 左右文字标签菜单组件 |
| `ui_Menu_With_Value.c` | 左侧名称 + 右侧数值菜单组件 |
| `ui_Menu_With_ValueSetting.c` | 可编辑数值设置菜单（点击进入编辑、上下箭头调节） |
| `ui_Menu_Blink.c` | 带文字闪烁效果的菜单组件 |
| `ui_Confirm_Dialog.c` | 模态确认对话框（多选项、长按确认） |
| `ui_Password_Dialog.c` | 7位数字密码输入对话框 |
| `ui_Prompt_Win.c` | 定时弹窗（自动关闭、点击关闭） |
| `ui_Prompt_Win_Center.c` | 居中弹窗菜单（用于快捷导航） |
| `ui_Gas_Menu.c` | 烟气测量5页显示界面（A/B/C/D/E页面） |
| `ui_Gas_Test_Menu.c` | 烟气测试程序2页显示界面 |
| `ui_Pressure_Menu.c` | 压力测量界面（数据列表页 + 图表页） |
| `ui_Temp_Diff_Menu.c` | 差温测量界面（图表页 + 详细数据页） |
| `ui_DateTime.c` | 日期时间编辑模块 |
| `ui_Last_Measurement.c` | 上一次测量数据保存/恢复 |
| `ui_Screen_Boot.c` | 开机动画和提示 |

### 1.2 菜单导航状态机

系统使用 `menu_state_t` 枚举管理所有界面状态，通过历史栈支持最多8层的前进/返回导航。

```c
// 前进到新菜单
ui_menu_navigate_to(MENU_STATE_PRESSURE);

// 返回上一级菜单（ESC键自动调用）
ui_menu_go_back();

// 获取当前菜单状态
menu_state_t state = ui_menu_get_current_state();
```

### 1.3 屏幕布局

```
┌──────────────────────────┐  y=0
│  顶部状态栏 (50px)        │  背景图片 + 电量条 + 归零进度条
│  top_hint_label           │  顶部提示文字（如"测量菜单"）
├──────────────────────────┤  y=40  ← ui_Rod (红色装饰杆)
│                          │  y=50
│   ui_Main_Content        │
│   (480×700)              │  动态内容区域，所有菜单在此创建
│                          │
│                          │
├──────────────────────────┤  y=750
│  底部按键区 (40px)        │  三个功能按键（左/中/右）
│  bottom_btns[0..2]       │
└──────────────────────────┘  y=800
```

---

## 2. 按键系统

### 2.1 全局按键事件分发

`ui.c` 中的 `global_key_event_cb` 是所有按键事件的入口，按优先级分层处理：

**优先级1: 弹窗层**（拦截所有按键，防止穿透到背景）

| 弹窗类型 | UP/DOWN | LEFT/RIGHT | ENTER | ESC |
|---------|---------|------------|-------|-----|
| 居中弹窗 (Prompt_Win_Center) | 切换焦点 | — | 确认 | 关闭 |
| 确认对话框 (Confirm_Dialog) | 切换焦点 | 切换焦点 | 确认 | 关闭 |
| 密码对话框 (Password_Dialog) | 数字+/- | 焦点左右移 | 确认 | 关闭 |

**优先级2: 编辑模式**（拦截UP/DOWN/ENTER/ESC，防止焦点移走）

| 编辑界面 | UP | DOWN | ENTER/ESC |
|---------|-----|------|-----------|
| 日期时间编辑 | 增加值 | 减少值 | 退出编辑 |
| Value_Setting编辑 | 增加值 | 减少值 | 退出编辑 |

**优先级3: 全局导航**

| 按键 | 功能 |
|------|------|
| ESC | 返回上一级菜单 |
| HOME | 打开快捷导航弹窗 |
| LEFT/RIGHT | 测量界面左右翻页（烟气/压力/差温/测试程序） |

### 2.2 按键组 (lv_group)

系统创建了全局按键组 `g_keypad_group`，设置为默认组。底部按键在配置时自动加入该组：

```c
// 在 ui_init() 中初始化
g_keypad_group = lv_group_create();
lv_group_set_default(g_keypad_group);

// 关联你的物理按键输入设备驱动
lv_indev_t * indev = lv_indev_create();
lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
lv_indev_set_read_cb(indev, your_keypad_read_cb);
lv_indev_set_group(indev, g_keypad_group);
```

> **⚠️ 需要用户实现**: `your_keypad_read_cb` 函数，将硬件按键映射到 LVGL 键码（`LV_KEY_UP`, `LV_KEY_DOWN`, `LV_KEY_LEFT`, `LV_KEY_RIGHT`, `LV_KEY_ENTER`, `LV_KEY_ESC`, `LV_KEY_HOME`）。

---

## 3. 菜单组件 API

### 3.1 基础子菜单 — `ui_Sub_Menu`

灰色背景 + 底部渐变分隔线的基础菜单列表，支持点击高亮和禁用状态。

```c
lv_obj_t * ui_Sub_Menu_create_with_cb(
    lv_obj_t * parent,           // 父容器（通常是 ui_Main_Content）
    const int16_t * item_heights,// 每项高度数组
    int16_t item_spacing,        // 项间距
    int16_t start_y,             // 起始Y坐标
    const char ** names,         // 名称数组
    uint16_t item_count,         // 项数量
    lv_event_cb_t event_cb       // 点击回调
);
```

**使用案例 — 创建主菜单:**

```c
static void main_menu_item_cb(lv_event_t * e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    switch (index) {
        case 0: ui_menu_navigate_to(MENU_STATE_GAS_MEASURE); break;
        case 1: ui_menu_navigate_to(MENU_STATE_PRESSURE);    break;
        case 2: ui_menu_navigate_to(MENU_STATE_TEMP_DIFF);   break;
    }
}

void ui_create_main_menu(void) {
    const char * names[] = {"烟气测量", "压力测量", "差温测量"};
    const int16_t heights[] = {60, 60, 60};

    my_menu_cont = ui_Sub_Menu_create_with_cb(
        ui_Main_Content, heights, 0, 50,
        names, 3, main_menu_item_cb
    );
}
```

**锁定/解锁菜单项:**

```c
// 禁用第3项（灰色不可点击）
ui_Sub_Menu_set_item_locked(my_menu_cont, 3, true);

// 解锁
ui_Sub_Menu_set_item_locked(my_menu_cont, 3, false);
```

> **⚠️ 需要用户实现**: `event_cb` 回调函数中的业务逻辑（如菜单导航、启动测量等）。

---

### 3.2 带文字标签菜单 — `ui_Menu_With_Text`

左侧名称 + 右侧文字，适合显示键值对（如燃料参数、系统设置文字显示）。

```c
lv_obj_t * ui_Menu_With_Text_create_with_cb(
    lv_obj_t * parent,
    const int16_t * item_heights,
    int16_t item_spacing,
    int16_t start_y,
    const char ** left_texts,    // 左侧文字数组
    const char ** right_texts,   // 右侧文字数组（NULL表示无右侧文字）
    uint16_t item_count,
    lv_event_cb_t event_cb,
    const lv_font_t * custom_font // 自定义字体，NULL使用默认
);
```

**动态更新:**

```c
// 更新右侧文字
ui_Menu_Text_set(my_menu_cont, 2, "新值文字");

// 更新左侧文字
ui_Menu_Left_Text_set(my_menu_cont, 0, "新标题");

// 滚动到指定项
ui_Menu_Text_scroll_to_item(my_menu_cont, 5, LV_ANIM_ON);
```

**实时数据显示案例 — 服务值监控:**

```c
// 在定时器回调中刷新显示（每秒更新一次）
static void refresh_service_values_display(void) {
    char buf[32];
    for (uint8_t i = 0; i < 16; i++) {
        service_values_get_str(i, buf, sizeof(buf));
        ui_Menu_Text_set(my_menu_cont, i, buf);
    }
}
```

---

### 3.3 带数值菜单 — `ui_Menu_With_Value`

左侧名称 + 右侧整数值显示。

```c
lv_obj_t * ui_Menu_With_Value_create_with_cb(
    lv_obj_t * parent,
    const int16_t * item_heights,
    int16_t item_spacing,
    int16_t start_y,
    const char ** names,
    const int32_t * values,      // 初始值，MENU_NO_VALUE(2147483647)表示不显示
    uint16_t item_count,
    lv_event_cb_t event_cb
);

// 动态更新数值
void ui_Menu_Value_set(lv_obj_t * main_cont, uint16_t index, int32_t new_value);
```

---

### 3.4 数值设置菜单 — `ui_Value_Setting`

可编辑的数值设置菜单，支持整数/小数显示、上下箭头调节。

```c
// 创建菜单
ui_value_setting_t * ui_Value_Setting_create(
    lv_obj_t * parent,
    const char * title,
    int16_t start_y
);

// 自定义行高版本
ui_value_setting_t * ui_Value_Setting_create_ex(
    lv_obj_t * parent,
    const char * title,
    int16_t start_y,
    int16_t item_height
);

// 添加设置项，返回项索引(-1失败)
int8_t ui_Value_Setting_add_item(
    ui_value_setting_t * menu,
    const value_setting_item_config_t * config
);
```

**配置项结构体:**

```c
typedef struct {
    const char * title;        // 左侧标题
    const char * unit;         // 单位（未使用，保留）
    int32_t min_value;         // 最小值（乘以10^decimal_places后的整数）
    int32_t max_value;         // 最大值
    int32_t step;              // 步长
    int32_t default_value;     // 默认值
    uint8_t decimal_places;    // 小数位数（0=整数，1=0.1步进...）
    bool readonly;             // 只读项（不可编辑）
    bool show_value;           // 是否显示右侧数值
} value_setting_item_config_t;
```

**使用案例 — CO限制设置:**

```c
void ui_create_co_limit_menu(void) {
    co_limit_menu = ui_Value_Setting_create(ui_Main_Content, "CO限值", 200);

    // 标题行（只读，不显示值）
    value_setting_item_config_t title = {
        .title = "程序1", .readonly = true, .show_value = false
    };
    ui_Value_Setting_add_item(co_limit_menu, &title);

    // 可编辑数值项
    value_setting_item_config_t co = {
        .title = "CO限制 [ppm]",
        .min_value = 0,
        .max_value = 50000,
        .step = 100,
        .default_value = 8000,
        .decimal_places = 0,   // 整数
        .readonly = false,
        .show_value = true
    };
    ui_Value_Setting_add_item(co_limit_menu, &co);

    my_menu_cont = co_limit_menu->main_cont;
    ui_set_bottom_config("确认", "默认", "返回", co_limit_bottom_cb);
}
```

**使用案例 — 带小数的燃料参数:**

```c
// O2ref: 范围0.0~21.0，步长0.1
value_setting_item_config_t o2ref = {
    .title = "O2ref [%]",
    .min_value = 0,          // 0.0 * 10
    .max_value = 210,        // 21.0 * 10
    .step = 1,               // 0.1（步长基于内部整数）
    .default_value = 30,     // 3.0 * 10
    .decimal_places = 1,     // 1位小数
    .readonly = false,
    .show_value = true
};
```

**读写值:**

```c
// 获取浮点值
float o2ref = ui_Value_Setting_get_value(menu, 2);      // 返回 3.0

// 获取原始整数
int32_t raw = ui_Value_Setting_get_value_int(menu, 2);   // 返回 30

// 设置值（自动限幅）
ui_Value_Setting_set_value(menu, 2, 50);                 // 设为 5.0

// 编辑状态查询/控制
bool editing = ui_Value_Setting_is_editing(menu);
ui_Value_Setting_confirm(menu);   // 退出编辑模式
ui_Value_Setting_cancel(menu);    // 取消编辑

// 按键调节（由全局按键回调自动调用）
ui_Value_Setting_step_up(menu);   // 增加一个step
ui_Value_Setting_step_down(menu); // 减少一个step
```

**资源释放:**

```c
// 先删除LVGL对象，再释放结构体
if (menu->main_cont != NULL) {
    lv_obj_del(menu->main_cont);
    menu->main_cont = NULL;
}
ui_Value_Setting_cleanup(menu);
menu = NULL;
```

> **⚠️ 需要用户实现**: 底部按键回调中的"确认"逻辑（将设置值保存到NVS/Flash等持久化存储）。

---

### 3.5 带闪烁菜单 — `ui_Blink_Menu`

在标准子菜单基础上支持指定项文字黑红交替闪烁。

```c
lv_obj_t * ui_Blink_Menu_create_with_cb(
    lv_obj_t * parent,
    const int16_t * item_heights,
    int16_t item_spacing,
    int16_t start_y,
    const char ** names,
    uint16_t item_count,
    lv_event_cb_t event_cb,
    const bool * blink_flags     // true=该项闪烁，NULL=全部不闪烁
);
```

---

## 4. 测量界面 API

### 4.1 烟气测量显示 — `ui_Gas_Measure_Display`

5页数据显示界面（A/B/C/D/E），支持手势和按键左右滑动切换。

**页面A** (主界面): 7个功能块（参数名+单位+大字数值）
**页面B** (图表页): 4个功能块 + 内嵌折线图 + 刻度标签
**页面C**: 2个功能块
**页面D/E**: 各7个功能块

```c
// 创建
ui_gas_measure_display_t * ui_Gas_Measure_Display_create(lv_obj_t * parent);

// 设置各页面功能块
void ui_Gas_Measure_Display_set_page_a(menu, block_index, name, unit, value, value_color);
void ui_Gas_Measure_Display_set_page_b(menu, block_index, name, unit, value, value_color);
void ui_Gas_Measure_Display_set_page_c(menu, block_index, name, unit, value, value_color);
void ui_Gas_Measure_Display_set_page_d(menu, block_index, name, unit, value, value_color);
void ui_Gas_Measure_Display_set_page_e(menu, block_index, name, unit, value, value_color);

// 通用接口（page参数为字符: 'a'~'e'）
void ui_Gas_Measure_Display_set_block(menu, page, block_index, name, unit, value, value_color);
void ui_Gas_Measure_Display_set_value(menu, page, block_index, value, value_color);

// 图表操作（仅页面B）
void ui_Gas_Measure_Display_add_chart_point_b(menu, block_index, value);
void ui_Gas_Measure_Display_set_chart_range_b(menu, block_index, min, max);
void ui_Gas_Measure_Display_set_chart_scale_b(menu, block_index, max_val, mid_val, min_val);

// 翻页
void ui_Gas_Measure_Display_switch_page(menu, page);  // page: 'a'~'e'
char ui_Gas_Measure_Display_get_current_page(menu);

// 销毁
void ui_Gas_Measure_Display_delete(menu);
```

**实时数据显示案例 — 烟气测量数据更新:**

```c
/* ⚠️ 需要用户实现: 传感器数据采集任务，通过定时器或任务调用以下更新函数 */

// 方法1: 完整更新（名称+单位+值）
ui_Gas_Measure_Display_set_page_a(gas_display_menu, 0, "O2", "%", "20.95", 0);

// 方法2: 仅更新数值（高效，适合实时刷新）
ui_Gas_Measure_Display_set_value(gas_display_menu, 'a', 0, "20.95", 0);

// 方法3: 带颜色的值更新（橙色高亮）
ui_Gas_Measure_Display_set_value(gas_display_menu, 'e', 4, "1013.25", 0xFF8C00);

// 图表数据点追加（页面B，每次采样调用一次）
ui_Gas_Measure_Display_add_chart_point_b(gas_display_menu, 0, sensor_value);
```

**完整初始化案例:**

```c
void ui_create_gas_program_menu(void) {
    gas_display_menu = ui_Gas_Measure_Display_create(ui_Main_Content);
    my_menu_cont = gas_display_menu->main_cont;

    // 初始化页面A
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 0, "O2",     "%",   "20.95", 0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 1, "CO",     "ppm", "0",     0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 2, "NO",     "ppm", "0",     0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 3, "NO2",    "ppm", "0",     0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 4, "NOx",    "ppm", "0",     0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 5, "T-烟气", "°C",  "--.-",  0);
    ui_Gas_Measure_Display_set_page_a(gas_display_menu, 6, "T-空气", "°C",  "--.-",  0);

    // 初始化页面B图表
    ui_Gas_Measure_Display_set_chart_range_b(gas_display_menu, 0, 0, 500);
    ui_Gas_Measure_Display_set_chart_scale_b(gas_display_menu, 0, "500", "250", "0");

    ui_set_top_hint("程序1,天然气");
    ui_set_bottom_config("开始", "保存/返回", "保持压力", gas_program_bottom_cb);
}
```

> **⚠️ 需要用户实现**:
> - 数据采集定时器（读取传感器 → 格式化字符串 → 调用 `set_value` 更新UI）
> - `gas_program_bottom_cb` 中的"开始/停止"逻辑（启停数据采集）
> - "保存/返回"逻辑（保存测量数据到存储）

---

### 4.2 烟气测试程序 — `ui_Test_Program`

2页测试程序数据显示，每页7个功能块，支持左右滑动循环切换。

```c
ui_test_program_t * ui_Test_Program_create(lv_obj_t * parent);

// 设置功能块内容
void ui_Test_Program_set_block(menu, page, block_index, name, unit, value, value_color);

// 仅更新数值（快速刷新）
void ui_Test_Program_set_value(menu, page, block_index, value, value_color);

// 翻页
void ui_Test_Program_switch_page(menu, page);  // page: 0或1
uint8_t ui_Test_Program_get_current_page(menu);

void ui_Test_Program_delete(menu);
```

**实时数据更新案例:**

```c
/* ⚠️ 需要用户实现: 在传感器数据采集回调中调用 */
static void update_test_program_display(void) {
    char buf[16];

    // 更新O2
    snprintf(buf, sizeof(buf), "%.2f", sensor_data.o2);
    ui_Test_Program_set_value(test_menu, 0, 0, buf, 0);

    // 更新CO
    snprintf(buf, sizeof(buf), "%d", (int)sensor_data.co);
    ui_Test_Program_set_value(test_menu, 0, 1, buf, 0);

    // 更新Pressure（绿色）
    snprintf(buf, sizeof(buf), "%.2f", sensor_data.pressure);
    ui_Test_Program_set_value(test_menu, 0, 6, buf, 0x008000);
}
```

---

### 4.3 压力测量 — `ui_Pressure_Menu`

2页：数据列表页（6行数据 + 数显区）+ 图表页（折线图 + peak值 + 数显区）。

```c
ui_pressure_menu_t * ui_Pressure_Menu_create(lv_obj_t * parent);

// 设置数据行（行索引0~3）
void ui_Pressure_Menu_set_row(menu, row_index, left_text, right_text);

// 设置峰值（两页同步更新）
void ui_Pressure_Menu_set_peak(menu, value);

// 设置数显区（两页同步更新）
void ui_Pressure_Menu_set_display(menu, unit, value);

// 图表追加数据点
void ui_Pressure_Menu_add_chart_point(menu, value);

// 翻页
void ui_Pressure_Menu_next_page(menu);
void ui_Pressure_Menu_prev_page(menu);
void ui_Pressure_Menu_switch_page(menu, page);

void ui_Pressure_Menu_delete(menu);
```

**实时压力数据显示案例:**

```c
/* ⚠️ 需要用户实现: 压力传感器采集定时器 */
static void pressure_sensor_timer_cb(lv_timer_t * timer) {
    // 1. 读取传感器
    float pressure = read_pressure_sensor();  // 用户实现

    // 2. 更新数据行
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", pressure);
    ui_Pressure_Menu_set_row(pressure_menu, 0, "差压", buf);

    // 3. 更新数显区
    ui_Pressure_Menu_set_display(pressure_menu, "Pa", buf);

    // 4. 追加图表数据点（需要缩放到图表Y轴范围）
    ui_Pressure_Menu_add_chart_point(pressure_menu, (int32_t)(pressure * 10));

    // 5. 更新峰值
    static float peak = 0;
    if (fabs(pressure) > fabs(peak)) {
        peak = pressure;
        snprintf(buf, sizeof(buf), "%.2f", peak);
        ui_Pressure_Menu_set_peak(pressure_menu, buf);
    }
}
```

---

### 4.4 差温测量 — `ui_Temp_Diff_Menu`

2页：图表页（双曲线T1/T2 + 温度差）+ 详细数据页。

```c
ui_temp_diff_menu_t * ui_Temp_Diff_Menu_create(lv_obj_t * parent);

// 设置温度值
void ui_Temp_Diff_Menu_set_t1(menu, value);    // 出口温度（蓝色）
void ui_Temp_Diff_Menu_set_t2(menu, value);    // 入口温度（红色）
void ui_Temp_Diff_Menu_set_diff(menu, value);  // 温度差（绿色）

// 图表操作
void ui_Temp_Diff_Menu_add_chart_point(menu, t1_value, t2_value);
void ui_Temp_Diff_Menu_set_chart_range(menu, min, max);

// 翻页
void ui_Temp_Diff_Menu_next_page(menu);
void ui_Temp_Diff_Menu_prev_page(menu);

void ui_Temp_Diff_Menu_delete(menu);
```

**实时差温数据显示案例:**

```c
/* ⚠️ 需要用户实现: 温度传感器采集定时器 */
static void temp_sensor_timer_cb(lv_timer_t * timer) {
    float t1 = read_thermocouple_1();  // 用户实现
    float t2 = read_thermocouple_2();  // 用户实现
    float diff = t2 - t1;

    char buf[16];

    snprintf(buf, sizeof(buf), "%.1f", t1);
    ui_Temp_Diff_Menu_set_t1(temp_diff_menu, buf);

    snprintf(buf, sizeof(buf), "%.1f", t2);
    ui_Temp_Diff_Menu_set_t2(temp_diff_menu, buf);

    snprintf(buf, sizeof(buf), "%.1f", diff);
    ui_Temp_Diff_Menu_set_diff(temp_diff_menu, buf);

    // 追加图表数据（int32_t，建议乘10保留1位小数精度）
    ui_Temp_Diff_Menu_add_chart_point(temp_diff_menu,
        (int32_t)(t1 * 10), (int32_t)(t2 * 10));
}
```

---

## 5. 对话框与弹窗 API

### 5.1 确认对话框 — `ui_Confirm_Dialog`

模态对话框，短按切换焦点，长按确认。

```c
typedef void (*confirm_dialog_cb_t)(uint8_t selected_index);

void ui_Confirm_Dialog_create(
    const char * title,
    const char ** options,       // 选项文字数组
    uint8_t num_options,
    uint8_t default_focus,       // 默认聚焦项
    confirm_dialog_cb_t callback // 确认回调
);

void ui_Confirm_Dialog_close(void);
bool ui_Confirm_Dialog_is_open(void);
```

**使用案例 — 关机确认:**

```c
static void shutdown_dialog_cb(uint8_t index) {
    if (index == 1) {
        // ⚠️ 需要用户实现: 实际关机逻辑
        // system_shutdown();
    }
    ui_Confirm_Dialog_close();
}

void show_shutdown_dialog(void) {
    const char * options[] = {"否,返回", "是,关机"};
    ui_Confirm_Dialog_create("关闭仪器?", options, 2, 0, shutdown_dialog_cb);
}
```

> **⚠️ 需要用户实现**: 回调函数中的实际业务逻辑（关机、恢复默认、删除数据等）。

---

### 5.2 密码对话框 — `ui_Password_Dialog`

7位数字密码输入，逐位输入，上下箭头调节。

```c
typedef void (*password_dialog_cb_t)(const char * password, bool complete);

void ui_Password_Dialog_create(
    const char * title,
    const char * subtitle,
    password_dialog_cb_t callback
);

void ui_Password_Dialog_close(void);
bool ui_Password_Dialog_is_open(void);
```

**使用案例:**

```c
void password_result_cb(const char * password, bool complete) {
    if (complete) {
        // ⚠️ 需要用户实现: 密码校验逻辑
        if (strcmp(password, "1234567") == 0) {
            ui_Password_Dialog_close();
            ui_menu_navigate_to(MENU_STATE_SERVICE_MENU);
        } else {
            // 密码错误处理
        }
    }
}

// 打开密码对话框
ui_Password_Dialog_create("服务密码", "输入密码", password_result_cb);
```

---

### 5.3 提示弹窗 — `ui_Prompt_Win`

带折角装饰的弹窗，支持定时自动关闭和点击关闭。使用队列管理多个弹窗。

```c
typedef void (*prompt_content_cb_t)(lv_obj_t * parent);

// 入队（推荐使用，支持排队显示）
void ui_Prompt_Win_enqueue(uint32_t timeout_ms, prompt_content_cb_t content_cb);

// 直接创建（会覆盖当前弹窗）
void ui_Prompt_Win_create(uint32_t timeout_ms, prompt_content_cb_t content_cb);

void ui_Prompt_Win_close(void);
```

**使用案例 — 自定义弹窗内容:**

```c
void build_custom_prompt(lv_obj_t * parent) {
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, "操作成功!\n\n数据已保存");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, font_normal_32, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -22);
}

// 显示3秒后自动关闭
ui_Prompt_Win_enqueue(3000, build_custom_prompt);
```

---

### 5.4 居中导航弹窗 — `ui_Prompt_Win_Center`

屏幕居中的选项弹窗，用于快捷导航。短按切换焦点，长按确认。

```c
typedef void (*prompt_win_center_cb_t)(uint16_t index);

void ui_Prompt_Win_Center_create(
    const int16_t * item_heights,
    const char ** names,
    uint16_t num_items,
    uint16_t default_focus,
    prompt_win_center_cb_t callback
);

void ui_Prompt_Win_Center_close(void);
bool ui_Prompt_Win_Center_is_open(void);
```

---

## 6. 顶部提示与底部按键 API

### 6.1 顶部提示

```c
// 设置顶部提示文字（NULL或""隐藏）
void ui_set_top_hint(const char * text);
```

### 6.2 底部按键

```c
typedef void (*bottom_btn_cb_t)(uint8_t index);  // index: 0=左, 1=中, 2=右

// 配置底部按键文字和回调（NULL或""隐藏该按键）
void ui_set_bottom_config(
    const char * left_txt,
    const char * mid_txt,
    const char * right_txt,
    bottom_btn_cb_t callback
);
```

**使用案例:**

```c
static void my_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 左键逻辑 */  break;
        case 1: /* 中键逻辑 */  break;
        case 2: /* 右键 - 返回 */
            ui_menu_navigate_to(MENU_STATE_MAIN);
            break;
    }
}

ui_set_top_hint("压力测量");
ui_set_bottom_config("接受", "零点", "保存/返回", my_bottom_cb);
```

> **⚠️ 需要用户实现**: `bottom_btn_cb_t` 回调函数中每个按键的具体业务逻辑。

---

## 7. 日期时间模块 API

```c
// 创建日期时间编辑菜单
lv_obj_t * ui_DateTime_Menu_create(lv_obj_t * parent, int16_t start_y);

// 获取/设置时间
void ui_DateTime_get(uint16_t * year, uint8_t * month, uint8_t * day,
                     uint8_t * hour, uint8_t * minute, uint8_t * second);
void ui_DateTime_set(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t minute, uint8_t second);

// 定时器控制
void ui_DateTime_timer_init(void);
void ui_DateTime_timer_pause(void);
void ui_DateTime_timer_resume(void);

// 编辑状态
bool ui_DateTime_is_editing(void);
void ui_DateTime_confirm_edit(void);
void ui_DateTime_step_up(void);    // 按键上调
void ui_DateTime_step_down(void);  // 按键下调
```

> **⚠️ 需要用户实现**: 在 `ui_DateTime_confirm_edit()` 中添加 RTC 写入逻辑（标记为TODO）。启动时从RTC读取时间调用 `ui_DateTime_set()` 同步。

---

## 8. 上一次测量数据 API

保存和恢复完整的测量快照（含图表数据）。

```c
// 保存测量数据
void last_measurement_save_from_gas_program(ui_gas_measure_display_t * menu);
void last_measurement_save_from_test_program(ui_test_program_t * menu);

// 恢复测量数据
void last_measurement_restore_to_gas_program(ui_gas_measure_display_t * menu);
void last_measurement_restore_to_test_program(ui_test_program_t * menu);

// 解锁主菜单的"上一个测量值"选项
void last_measurement_unlock_menu_item(void);

// 检查是否有有效数据
bool last_measurement_is_valid(void);
```

---

## 9. 零点校准 API

```c
bool ui_zero_calibration_start(void);       // 启动校准
bool ui_zero_calibration_is_running(void);   // 查询状态
void ui_zero_calibration_stop(void);         // 停止校准
```

> **⚠️ 需要用户实现**: 当前校准逻辑是模拟的进度条动画。需要替换为实际的传感器零点校准流程。

---

## 10. 系统设置 API

```c
// 获取/重置设置
system_settings_t * system_settings_get_current(void);
void system_settings_reset_to_default(void);

// 状态转字符串（用于菜单显示）
const char * bluetooth_state_to_str(bluetooth_state_t state);
const char * help_tip_state_to_str(help_tip_state_t state);
// ...等
```

> **⚠️ 需要用户实现**: 设置菜单中各项的实际功能（蓝牙开关、LCD亮度调节、语言切换等），以及设置的NVS持久化存储。

---

## 11. 需要用户实现的自定义回调汇总

### 11.1 传感器数据采集（关键）

| 位置 | 功能 | 当前状态 |
|------|------|---------|
| `ui_menu_manager.c` → `gas_program_menu_bottom_cb` | 烟气测量开始/停止 | 注释 `// start_gas_measurement()` |
| `ui_menu_manager.c` → `gas_test_program_menu_bottom_cb` | 测试程序开始/停止 | 仅切换按钮文字 |
| `ui_menu_manager.c` → `pressure_menu_bottom_cb` | 压力测量接受/零点 | case 0/1 为空 |
| `ui_menu_manager.c` → `temp_diff_menu_bottom_cb` | 差温测量restart | 仅重置UI显示 |

建议实现方式：创建LVGL定时器，在定时器回调中读取传感器并调用对应的 `set_value` / `add_chart_point` 等API更新UI。

### 11.2 数据存储

| 位置 | 功能 | 当前状态 |
|------|------|---------|
| `ui_menu_manager.c` → `co_limit_menu_bottom_cb` case 0 | 保存CO限制值 | 注释 `// config_set_co_limit(...)` |
| `ui_menu_manager.c` → 设置菜单 | 保存系统设置 | 仅修改内存变量 |
| `ui_menu_manager.c` → 场地管理 | 场地数据CRUD | 使用模拟数组 |
| `ui_DateTime.c` → `ui_DateTime_confirm_edit` | 写入RTC时间 | 注释 `// rtc_set_datetime(...)` |

### 11.3 硬件交互

| 位置 | 功能 | 当前状态 |
|------|------|---------|
| `ui.c` → `shutdown_dialog_cb` | 系统关机 | 注释 `// system_shutdown()` |
| `ui.c` → `recovery_dialog_cb` | 恢复出厂设置 | 仅重置内存 |
| `ui.c` → `zero_bar_timer_cb` | 零点校准流程 | 模拟进度条 |
| `ui.c` → `password_result_cb` | 密码验证 | 仅打印日志 |
| `ui.c` → `ui_init` | 按键输入设备驱动 | 需要用户添加 `lv_indev` |

### 11.4 按键输入设备驱动（必须实现）

```c
/* 在 ui_init() 之后，用户需要注册按键输入设备 */
static void keypad_read_cb(lv_indev_t * indev, lv_indev_data_t * data) {
    // ⚠️ 需要用户实现: 读取硬件按键状态
    uint8_t key = read_hardware_key();  // 用户硬件驱动

    if (key != 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        switch (key) {
            case HW_KEY_UP:    data->key = LV_KEY_UP;    break;
            case HW_KEY_DOWN:  data->key = LV_KEY_DOWN;  break;
            case HW_KEY_LEFT:  data->key = LV_KEY_LEFT;  break;
            case HW_KEY_RIGHT: data->key = LV_KEY_RIGHT; break;
            case HW_KEY_ENTER: data->key = LV_KEY_ENTER; break;
            case HW_KEY_ESC:   data->key = LV_KEY_ESC;   break;
            case HW_KEY_HOME:  data->key = LV_KEY_HOME;  break;
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void app_main(void) {
    // ... LVGL初始化 ...
    ui_init();

    // 注册按键输入设备
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keypad_read_cb);
    lv_indev_set_group(indev, g_keypad_group);

    // 加载主屏幕并创建主菜单
    lv_screen_load(ui_Screen_Main);
    ui_create_main_menu();
}
```

---

## 12. 实时数据显示完整集成案例

以下展示如何将传感器数据与UI系统完整集成：

```c
/* === 传感器数据结构 === */
typedef struct {
    float o2;
    float co;
    float no;
    float t_gas;
    float t_air;
    float pressure;
} sensor_data_t;

static sensor_data_t g_sensor_data = {0};
static lv_timer_t * g_sensor_timer = NULL;

/* === 传感器数据采集（⚠️ 用户实现硬件读取） === */
static void read_all_sensors(sensor_data_t * data) {
    // data->o2 = adc_read_o2_sensor();
    // data->co = adc_read_co_sensor();
    // data->no = adc_read_no_sensor();
    // data->t_gas = spi_read_thermocouple_gas();
    // data->t_air = spi_read_thermocouple_air();
    // data->pressure = i2c_read_pressure_sensor();
}

/* === 定时器回调 - 刷新UI === */
static void sensor_update_timer_cb(lv_timer_t * timer) {
    read_all_sensors(&g_sensor_data);
    char buf[16];

    if (current_menu_state == MENU_STATE_GAS_PROGRAM && gas_display_menu != NULL) {
        snprintf(buf, sizeof(buf), "%.2f", g_sensor_data.o2);
        ui_Gas_Measure_Display_set_value(gas_display_menu, 'a', 0, buf, 0);

        snprintf(buf, sizeof(buf), "%d", (int)g_sensor_data.co);
        ui_Gas_Measure_Display_set_value(gas_display_menu, 'a', 1, buf, 0);

        snprintf(buf, sizeof(buf), "%.1f", g_sensor_data.t_gas);
        ui_Gas_Measure_Display_set_value(gas_display_menu, 'a', 5, buf, 0);

        // 页面B图表追加数据点
        ui_Gas_Measure_Display_add_chart_point_b(gas_display_menu, 0,
            (int32_t)(g_sensor_data.t_gas * 10));
    }

    if (current_menu_state == MENU_STATE_PRESSURE && pressure_menu != NULL) {
        snprintf(buf, sizeof(buf), "%.1f", g_sensor_data.pressure);
        ui_Pressure_Menu_set_display(pressure_menu, "Pa", buf);
        ui_Pressure_Menu_set_row(pressure_menu, 0, "差压", buf);
        ui_Pressure_Menu_add_chart_point(pressure_menu,
            (int32_t)(g_sensor_data.pressure));
    }
}

/* === 开始/停止测量 === */
void start_measurement(void) {
    if (g_sensor_timer == NULL) {
        g_sensor_timer = lv_timer_create(sensor_update_timer_cb, 1000, NULL);
    }
}

void stop_measurement(void) {
    if (g_sensor_timer != NULL) {
        lv_timer_del(g_sensor_timer);
        g_sensor_timer = NULL;
    }
}

/* === 在菜单回调中调用 === */
static void gas_program_bottom_cb(uint8_t index) {
    switch (index) {
        case 0: /* 开始/停止 */
            if (gas_program_is_running) {
                stop_measurement();
                gas_program_is_running = false;
                last_measurement_save_from_gas_program(gas_display_menu);
                last_measurement_unlock_menu_item();
                ui_set_bottom_config("开始", "保存/返回", "保持压力",
                                     gas_program_bottom_cb);
            } else {
                start_measurement();
                gas_program_is_running = true;
                ui_set_bottom_config("停止", "保存/返回", "保持压力",
                                     gas_program_bottom_cb);
            }
            break;
        case 1: /* 保存/返回 */
            stop_measurement();
            ui_menu_navigate_to(MENU_STATE_GAS_MEASURE);
            break;
    }
}
```

---

## 13. 进度条 API

```c
// 归零进度条（顶部状态栏）
void set_zero_bar_value(uint8_t value);   // 0~100

// 电量进度条（顶部状态栏）
void set_charge_bar_value(uint8_t value); // 0~100
```

> **⚠️ 需要用户实现**: 电池电量读取任务，定期调用 `set_charge_bar_value()` 更新电量显示。


---

## 14. 如何修改开机动画和字体

> **⚠️ 需要用户实现**: 使用lvgl官方的字体转换工具和图片转换工具转换成C数组，并且替换原文件的lv_img_logo以及my_Font_xx。