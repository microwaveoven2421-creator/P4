/**
 * @file ui.h
 *
 * @brief UI初始化函数和语言设置函数
 *
 * @author MO
 * @date 2026-04-21
 *
 * @version 1.0
 *
**/
#ifndef UI_H
#define UI_H

void ui_init(void);

// 语言
typedef enum {
    LANG_ZH = 0,
    LANG_EN
} ui_lang_t;

void ui_set_language(ui_lang_t lang);
ui_lang_t ui_get_language(void);

#endif