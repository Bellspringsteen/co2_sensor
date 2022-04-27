#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "file_action";

int read_co2(){
    FILE* f = fopen("/spiffs/co2", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return 0;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    return atoi(line);
}

void write_co2(int co2){
    FILE* f = fopen("/spiffs/co2", "r");
    if (f == NULL) {
        ESP_LOGE("BUTTON", "Failed to open file for writing");
        return;
    }
    fprintf(f,"%d\n", co2);
    fclose(f);
}
