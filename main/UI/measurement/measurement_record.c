#include "measurement_record.h"
#include "UI/ui_manager.h"
#include "UI/ui.h"
#include "comp/ui_confirm.h"
#include "ui_common.h"
#include "screens/ui_setting.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define SPIFFS_BASE_PATH "/spiffs"

typedef struct {
    lv_obj_t *start_label;
    bool running;
    measurement_kind_t kind;
    const char *title;
    measurement_snapshot_cb_t snapshot_cb;
    void *snapshot_user_data;
} measurement_bar_ctx_t;

static measurement_record_t g_records[MEASUREMENT_RECORD_MAX];
static uint16_t g_record_count;

static void build_date(char *date, size_t date_size, char *file_date, size_t file_date_size)
{
    time_t now = time(NULL);
    struct tm *tm_info;

    if(now < 1577836800) {
        struct tm fallback = {
            .tm_year = 2026 - 1900,
            .tm_mon = 4 - 1,
            .tm_mday = 24,
            .tm_hour = 12,
            .tm_min = 0,
            .tm_sec = 0
        };
        now = mktime(&fallback) + g_record_count;
    }

    tm_info = localtime(&now);
    if(!tm_info) {
        snprintf(date, date_size, "2026-04-24 12:00:00");
        snprintf(file_date, file_date_size, "20260424_120000");
        return;
    }

    strftime(date, date_size, "%Y-%m-%d %H:%M:%S", tm_info);
    strftime(file_date, file_date_size, "%Y%m%d_%H%M%S", tm_info);
}

static measurement_record_t *next_record_slot(void)
{
    if(g_record_count < MEASUREMENT_RECORD_MAX) {
        return &g_records[g_record_count++];
    }

    memmove(&g_records[0], &g_records[1], sizeof(g_records[0]) * (MEASUREMENT_RECORD_MAX - 1));
    return &g_records[MEASUREMENT_RECORD_MAX - 1];
}

static void write_record_file(const measurement_record_t *record)
{
    FILE *fp = fopen(record->filename, "w");

    if(!fp) {
        return;
    }

    fprintf(fp, "%s\n%s: %s\n", record->title, ui_lang("日期", "Date"), record->date);
    if(record->has_gps) {
        fprintf(fp, "%s: %s, %s\n", ui_lang("经纬度坐标", "GPS"), record->latitude, record->longitude);
    }
    fprintf(fp, "\n%s\n", record->content);
    fclose(fp);
}

static void show_saved_dialog(const measurement_record_t *record)
{
    char message[256];

    if(record->has_gps) {
        snprintf(message, sizeof(message),
                 "%s\n%s: %s\n%s: %s, %s",
                 ui_lang("已保存", "Saved"),
                 ui_lang("日期", "Date"),
                 record->date,
                 ui_lang("经纬度坐标", "GPS"),
                 record->latitude,
                 record->longitude);
    }
    else {
        snprintf(message, sizeof(message),
                 "%s\n%s: %s",
                 ui_lang("已保存", "Saved"),
                 ui_lang("日期", "Date"),
                 record->date);
    }

    ui_confirm_show_ok(message, NULL, NULL);
}

static void back_event(lv_event_t *e)
{
    (void)e;
    ui_menu_back();
}

static void start_event(lv_event_t *e)
{
    measurement_bar_ctx_t *ctx = lv_event_get_user_data(e);

    if(!ctx) {
        return;
    }

    ctx->running = !ctx->running;
    lv_label_set_text(ctx->start_label,
                      ui_lang(ctx->running ? "\xE5\x81\x9C\xE6\xAD\xA2" : "\xE5\xBC\x80\xE5\xA7\x8B",
                              ctx->running ? "Stop" : "Start"));
}

static void save_event(lv_event_t *e)
{
    measurement_bar_ctx_t *ctx = lv_event_get_user_data(e);
    const measurement_record_t *record;

    if(!ctx) {
        return;
    }

    record = measurement_record_save(ctx->kind, ctx->title, ctx->snapshot_cb, ctx->snapshot_user_data);
    if(record) {
        show_saved_dialog(record);
    }
}

static void print_event(lv_event_t *e)
{
    (void)e;
    ui_confirm_show_ok(ui_lang("\xE6\x89\x93\xE5\x8D\xB0\xE5\x8A\x9F\xE8\x83\xBD\xE5\xBE\x85\xE6\x8E\xA5\xE5\x85\xA5",
                              "Print is not connected yet"),
                       NULL,
                       NULL);
}

static void bar_delete_event(lv_event_t *e)
{
    measurement_bar_ctx_t *ctx = lv_event_get_user_data(e);

    (void)e;
    if(ctx) {
        lv_free(ctx);
    }
}

static lv_obj_t *create_bottom_btn(lv_obj_t *parent, const char *text)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *label = lv_label_create(btn);

    lv_obj_set_size(btn, 124, 52);
    lv_obj_set_style_radius(btn, 18, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCE8F6), LV_STATE_PRESSED);

    lv_label_set_text(label, text);
    ui_apply_btn_text_style(label);
    lv_obj_center(label);

    return btn;
}

void measurement_record_init(void)
{
    g_record_count = 0;
}

const char *measurement_kind_name(measurement_kind_t kind)
{
    switch(kind) {
        case MEASUREMENT_KIND_GAS:
            return ui_lang("\xE7\x83\x9F\xE6\xB0\x94\xE6\xB5\x8B\xE9\x87\x8F", "Gas Measurement");
        case MEASUREMENT_KIND_TEMP_DIFF:
            return ui_lang("\xE6\xB8\xA9\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F", "Temperature Difference");
        case MEASUREMENT_KIND_PRESSURE:
            return ui_lang("\xE5\x8E\x8B\xE5\xB7\xAE\xE6\xB5\x8B\xE9\x87\x8F", "Differential Pressure");
        default:
            return ui_lang("\xE6\xB5\x8B\xE9\x87\x8F", "Measurement");
    }
}

const measurement_record_t *measurement_record_save(measurement_kind_t kind,
                                                    const char *title,
                                                    measurement_snapshot_cb_t snapshot_cb,
                                                    void *user_data)
{
    measurement_record_t *record = next_record_slot();
    char file_date[24];

    memset(record, 0, sizeof(*record));
    build_date(record->date, sizeof(record->date), file_date, sizeof(file_date));
    snprintf(record->title, sizeof(record->title), "%s", title ? title : measurement_kind_name(kind));
    snprintf(record->filename, sizeof(record->filename), SPIFFS_BASE_PATH"/measurement_%s.txt", file_date);

    if(ui_setting_gps_enabled()) {
        record->has_gps = true;
        snprintf(record->latitude, sizeof(record->latitude), "%s", "31.2304N");
        snprintf(record->longitude, sizeof(record->longitude), "%s", "121.4737E");
    }

    if(snapshot_cb) {
        snapshot_cb(record->content, sizeof(record->content), user_data);
    }
    else {
        snprintf(record->content, sizeof(record->content), "%s", measurement_kind_name(kind));
    }

    write_record_file(record);

    return record;
}

uint16_t measurement_record_count(void)
{
    return g_record_count;
}

const measurement_record_t *measurement_record_get(uint16_t index)
{
    if(index >= g_record_count) {
        return NULL;
    }

    return &g_records[index];
}

lv_obj_t *measurement_create_top_back(lv_obj_t *parent)
{
    return ui_create_back_btn(parent, back_event);
}

lv_obj_t *measurement_create_bottom_bar(lv_obj_t *parent,
                                        measurement_kind_t kind,
                                        const char *title,
                                        measurement_snapshot_cb_t snapshot_cb,
                                        void *user_data)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_t *start_btn;
    lv_obj_t *save_btn;
    lv_obj_t *print_btn;
    measurement_bar_ctx_t *ctx = lv_malloc(sizeof(*ctx));

    if(ctx) {
        memset(ctx, 0, sizeof(*ctx));
        ctx->kind = kind;
        ctx->title = title;
        ctx->snapshot_cb = snapshot_cb;
        ctx->snapshot_user_data = user_data;
    }

    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, lv_pct(100), 82);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xEAF0F6), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0xD7E0EA), 0);
    lv_obj_set_style_pad_left(bar, 16, 0);
    lv_obj_set_style_pad_right(bar, 16, 0);
    lv_obj_set_style_pad_top(bar, 10, 0);
    lv_obj_set_style_pad_bottom(bar, 10, 0);
    lv_obj_set_style_pad_column(bar, 12, 0);
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    start_btn = create_bottom_btn(bar, ui_lang("\xE5\xBC\x80\xE5\xA7\x8B", "Start"));
    save_btn = create_bottom_btn(bar, ui_lang("\xE4\xBF\x9D\xE5\xAD\x98", "Save"));
    print_btn = create_bottom_btn(bar, ui_lang("\xE6\x89\x93\xE5\x8D\xB0", "Print"));

    if(ctx) {
        ctx->start_label = lv_obj_get_child(start_btn, 0);
        lv_obj_add_event_cb(start_btn, start_event, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(save_btn, save_event, LV_EVENT_CLICKED, ctx);
        lv_obj_add_event_cb(bar, bar_delete_event, LV_EVENT_DELETE, ctx);
    }

    lv_obj_add_event_cb(print_btn, print_event, LV_EVENT_CLICKED, NULL);

    (void)print_btn;
    return bar;
}
