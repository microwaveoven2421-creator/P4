#include "ui.h"
#include "ui_manager.h"
#include "comp/ui_input.h"
#include "comp/ui_date_picker.h"

static ui_lang_t g_lang = LANG_ZH;

ui_lang_t ui_get_language(void)
{
    return g_lang;
}

void ui_set_language(ui_lang_t lang)
{
    g_lang = lang;
}

void ui_init(void)
{
    ui_input_init(lv_layer_top());
    ui_date_picker_init(lv_layer_top());
    ui_manager_init();
}
