#include "screens/ui_measurement_unit.h"
#include "ui_common.h"
#include "UI/ui_manager.h"

#include <stdint.h>

typedef struct {
    const char *zh;
    const char *en;
} unit_option_t;

typedef struct {
    const char *label_zh;
    const char *label_en;
    const unit_option_t *options;
    uint16_t option_count;
} measurement_unit_item_t;

enum {
    UNIT_ITEM_TEMPERATURE = 0,
    UNIT_ITEM_DIFF_PRESSURE,
    UNIT_ITEM_PRESSURE,
    UNIT_ITEM_P_ABS,
    UNIT_ITEM_MASS_FLOW,
    UNIT_ITEM_UNIT_DENSITY,
    UNIT_ITEM_MASS_ENERGY,
    UNIT_ITEM_MASS_CAPACITY,
    UNIT_ITEM_O2_RESOLUTION,
    UNIT_ITEM_CORE_FLOW_SEARCH,
    UNIT_ITEM_ASSIST,
    UNIT_ITEM_COUNT
};

static const unit_option_t temperature_options[] = {
    {"°C", "°C"},
    {"°F", "°F"},
};

static const unit_option_t pressure_options[] = {
    {"mmH2O", "mmH2O"},
    {"cmH2O", "cmH2O"},
    {"inchH2O", "inchH2O"},
    {"mmHg", "mmHg"},
    {"nHg", "nHg"},
    {"PSI", "PSI"},
    {"bar", "bar"},
    {"MPa", "MPa"},
    {"Pa", "Pa"},
    {"hPa/Pa", "hPa/Pa"},
    {"kPa", "kPa"},
    {"mbar", "mbar"},
};

static const unit_option_t mass_flow_options[] = {
    {"mg/s", "mg/s"},
    {"lb/yr", "lb/yr"},
    {"Tpy", "Tpy"},
};

static const unit_option_t unit_density_options[] = {
    {"mg/Nm\xC2\xB3", "mg/Nm\xC2\xB3"},
    {"grain/ft\xC2\xB3", "grain/ft\xC2\xB3"},
};

static const unit_option_t mass_energy_options[] = {
    {"mg/MJ", "mg/MJ"},
    {"lb/MMBtu", "lb/MMBtu"},
};

static const unit_option_t mass_capacity_options[] = {
    {"mg/KWh", "mg/KWh"},
    {"g/bhp-hr", "g/bhp-hr"},
};

static const unit_option_t o2_resolution_options[] = {
    {"0.01%", "0.01%"},
    {"0.1%", "0.1%"},
};

static const unit_option_t toggle_options[] = {
    {"\xE5\xBC\x80", "On"},
    {"\xE5\x85\xB3", "Off"},
};

static const measurement_unit_item_t measurement_unit_items[UNIT_ITEM_COUNT] = {
    {"\xE6\xB8\xA9\xE5\xBA\xA6\xE5\x8D\x95\xE4\xBD\x8D", "Temperature Unit", temperature_options, 2},
    {"\xE5\x8E\x8B\xE5\xB7\xAE\xE5\x8D\x95\xE4\xBD\x8D", "Differential Pressure Unit", pressure_options, 12},
    {"\xE5\x8E\x8B\xE5\x8A\x9B\xE5\x8D\x95\xE4\xBD\x8D", "Pressure Unit", pressure_options, 12},
    {"P-abs\xE5\x8D\x95\xE4\xBD\x8D", "P-abs Unit", pressure_options, 12},
    {"Mass flow", "Mass flow", mass_flow_options, 3},
    {"Unit density", "Unit density", unit_density_options, 2},
    {"Mass/Energy", "Mass/Energy", mass_energy_options, 2},
    {"Mass/Capacity", "Mass/Capacity", mass_capacity_options, 2},
    {"O2\xE5\x88\x86\xE8\xBE\xA8\xE7\x8E\x87", "O2 Resolution", o2_resolution_options, 2},
    {"core flow search", "core flow search", toggle_options, 2},
    {"assist", "assist", toggle_options, 2},
};

static uint16_t selected_indices[UNIT_ITEM_COUNT] = {0};
static lv_obj_t *value_labels[UNIT_ITEM_COUNT];

static bool value_label_ready(uint16_t item_index)
{
    return item_index < UNIT_ITEM_COUNT &&
           value_labels[item_index] &&
           lv_obj_is_valid(value_labels[item_index]);
}

static const char *get_option_text(uint16_t item_index)
{
    const measurement_unit_item_t *item = &measurement_unit_items[item_index];
    const unit_option_t *option = &item->options[selected_indices[item_index]];

    return ui_lang(option->zh, option->en);
}

static void refresh_value_label(uint16_t item_index)
{
    if(!value_label_ready(item_index)) {
        return;
    }

    lv_label_set_text(value_labels[item_index], get_option_text(item_index));
}

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void screen_delete_event(lv_event_t *e)
{
    uint16_t i;

    (void)e;

    for(i = 0; i < UNIT_ITEM_COUNT; i++) {
        value_labels[i] = NULL;
    }
}

static void unit_item_event(lv_event_t *e)
{
    uint16_t item_index = (uint16_t)(uintptr_t)lv_event_get_user_data(e);
    const measurement_unit_item_t *item;

    if(item_index >= UNIT_ITEM_COUNT) {
        return;
    }

    item = &measurement_unit_items[item_index];
    selected_indices[item_index] = (uint16_t)((selected_indices[item_index] + 1U) % item->option_count);
    refresh_value_label(item_index);
}

lv_obj_t *ui_measurement_unit_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list = NULL;
    uint16_t i;

    ui_create_title(screen,
                    ui_lang("\xE6\xB5\x8B\xE9\x87\x8F\xE5\x8D\x95\xE4\xBD\x8D",
                            "Measurement Units"));
    ui_create_back_btn(screen, back_event);
    list = ui_create_page_list(screen);

    for(i = 0; i < UNIT_ITEM_COUNT; i++) {
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);

        lv_obj_add_event_cb(btn, unit_item_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_label_set_text(label, ui_lang(measurement_unit_items[i].label_zh, measurement_unit_items[i].label_en));
        ui_apply_btn_text_style(label);
        lv_obj_set_width(label, 220);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        value_labels[i] = lv_label_create(btn);
        lv_obj_set_width(value_labels[i], 170);
        lv_label_set_long_mode(value_labels[i], LV_LABEL_LONG_DOT);
        refresh_value_label(i);
        ui_apply_btn_text_style(value_labels[i]);
        lv_obj_set_style_text_align(value_labels[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(value_labels[i], LV_ALIGN_RIGHT_MID, 0, 0);
    }

    lv_obj_add_event_cb(screen, screen_delete_event, LV_EVENT_DELETE, NULL);

    return screen;
}
