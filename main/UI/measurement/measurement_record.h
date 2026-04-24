#ifndef MEASUREMENT_RECORD_H
#define MEASUREMENT_RECORD_H

#include "lvgl.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MEASUREMENT_RECORD_MAX 16
#define MEASUREMENT_RECORD_TEXT_MAX 512

typedef enum {
    MEASUREMENT_KIND_GAS = 0,
    MEASUREMENT_KIND_TEMP_DIFF,
    MEASUREMENT_KIND_PRESSURE
} measurement_kind_t;

typedef struct {
    char filename[64];
    char title[48];
    char date[24];
    char latitude[24];
    char longitude[24];
    bool has_gps;
    char content[MEASUREMENT_RECORD_TEXT_MAX];
} measurement_record_t;

typedef void (*measurement_snapshot_cb_t)(char *buf, size_t buf_size, void *user_data);

void measurement_record_init(void);
const measurement_record_t *measurement_record_save(measurement_kind_t kind,
                                                    const char *title,
                                                    measurement_snapshot_cb_t snapshot_cb,
                                                    void *user_data);
uint16_t measurement_record_count(void);
const measurement_record_t *measurement_record_get(uint16_t index);
const char *measurement_kind_name(measurement_kind_t kind);

lv_obj_t *measurement_create_top_back(lv_obj_t *parent);
lv_obj_t *measurement_create_bottom_bar(lv_obj_t *parent,
                                        measurement_kind_t kind,
                                        const char *title,
                                        measurement_snapshot_cb_t snapshot_cb,
                                        void *user_data);

#endif
