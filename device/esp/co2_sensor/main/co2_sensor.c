#include "esp_log.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "sdkconfig.h"
#include <stdint.h>

#include "display_output.h"
#include "co2_scd_sensor.h"

#define TASK_PRI   4
#define TASK_STACK 8192 // 2048


static const char *TAG = "main";


void filesystem_init(){
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
}


void app_main(void)
{
    ESP_LOGE(TAG, "App Main");
    filesystem_init();
    
    xTaskCreate(led_display_task, "led Display", TASK_STACK, NULL, 3, NULL);
    xTaskCreate(co2_scd_task, "co2 sensor Task", TASK_STACK, NULL, 3, NULL);
}
