#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <scd30.h>
#include <esp_log.h>
#include <esp_err.h>
#include <file_actions.h>

static const char *TAG = "co2_scd_sensor";

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

void co2_scd_task(void *pvParameters)
{
    ESP_ERROR_CHECK(i2cdev_init());

    i2c_dev_t dev = {0};

    ESP_ERROR_CHECK(scd30_init_desc(&dev, 0, 15, 13));

    ESP_ERROR_CHECK(scd30_trigger_continuous_measurement(&dev, 0));

    float co2, temperature, humidity;
    bool data_ready;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));

        scd30_get_data_ready_status(&dev, &data_ready);

        if (data_ready)
        {
            esp_err_t res = scd30_read_measurement(&dev, &co2, &temperature, &humidity);
            if (res != ESP_OK)
            {
                ESP_LOGE(TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
                continue;
            }

            if (co2 == 0)
            {
                ESP_LOGW(TAG, "Invalid sample detected, skipping");
                continue;
            }

            ESP_LOGI(TAG, "CO2: %.0f ppm", co2);
            ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);
            ESP_LOGI(TAG, "Humidity: %.2f %%", humidity);

            write_co2(co2);
        }
    }
}
