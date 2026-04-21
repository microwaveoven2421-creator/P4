#include "ui.h"
#include "ui_manager.h"

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
    ui_manager_init();
}
