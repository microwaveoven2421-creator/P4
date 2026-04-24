#include "screens/ui_saved_files.h"
#include "UI/ui_manager.h"
#include "measurement/measurement_record.h"
#include "ui_common.h"

#include <stdint.h>
#include <stdio.h>

static uint16_t g_selected_record_index;

static void record_event(lv_event_t *e)
{
    uint16_t index = (uint16_t)(uintptr_t)lv_event_get_user_data(e);

    g_selected_record_index = index;
    ui_menu_navigate(UI_MENU_SAVED_FILE_DETAIL);
}

lv_obj_t *ui_saved_files_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;
    uint16_t count = measurement_record_count();

    ui_create_title(screen, ui_lang("\xE5\xB7\xB2\xE4\xBF\x9D\xE5\xAD\x98\xE6\x96\x87\xE4\xBB\xB6", "Saved Files"));
    ui_create_nav_bar(screen);
    list = ui_create_page_list(screen);

    if(count == 0) {
        lv_obj_t *label = lv_label_create(list);
        lv_label_set_text(label, ui_lang("\xE6\x9A\x82\xE6\x97\xA0\xE4\xBF\x9D\xE5\xAD\x98\xE6\x95\xB0\xE6\x8D\xAE", "No saved measurements"));
        ui_apply_btn_text_style(label);
        return screen;
    }

    for(uint16_t i = 0; i < count; i++) {
        const measurement_record_t *record = measurement_record_get(i);
        lv_obj_t *btn = ui_create_list_btn(list);
        lv_obj_t *label = lv_label_create(btn);
        char row[128];

        snprintf(row, sizeof(row), "%s  %s", record->date, record->title);
        lv_obj_add_event_cb(btn, record_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_label_set_text(label, row);
        ui_apply_btn_text_style(label);
        lv_obj_set_width(label, 390);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    }

    return screen;
}

lv_obj_t *ui_saved_file_detail_create(void)
{
    lv_obj_t *screen = ui_create_screen();
    lv_obj_t *list;
    const measurement_record_t *record = measurement_record_get(g_selected_record_index);

    ui_create_title(screen, ui_lang("\xE6\xB5\x8B\xE9\x87\x8F\xE6\x95\xB0\xE6\x8D\xAE", "Measurement Data"));
    ui_create_nav_bar(screen);
    list = ui_create_page_list(screen);

    if(!record) {
        lv_obj_t *label = lv_label_create(list);
        lv_label_set_text(label, ui_lang("\xE8\xAE\xB0\xE5\xBD\x95\xE4\xB8\x8D\xE5\xAD\x98\xE5\x9C\xA8", "Record not found"));
        ui_apply_btn_text_style(label);
        return screen;
    }

    {
        char text[MEASUREMENT_RECORD_TEXT_MAX + 192];
        lv_obj_t *label = lv_label_create(list);

        if(record->has_gps) {
            snprintf(text, sizeof(text),
                     "%s\n%s: %s\n%s: %s\n%s: %s, %s\n\n%s",
                     record->title,
                     ui_lang("文件", "File"),
                     record->filename,
                     ui_lang("日期", "Date"),
                     record->date,
                     ui_lang("经纬度坐标", "GPS"),
                     record->latitude,
                     record->longitude,
                     record->content);
        }
        else {
            snprintf(text, sizeof(text),
                     "%s\n%s: %s\n%s: %s\n\n%s",
                     record->title,
                     ui_lang("文件", "File"),
                     record->filename,
                     ui_lang("日期", "Date"),
                     record->date,
                     record->content);
        }

        lv_label_set_text(label, text);
        lv_obj_set_width(label, 400);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        ui_apply_btn_text_style(label);
    }

    return screen;
}
