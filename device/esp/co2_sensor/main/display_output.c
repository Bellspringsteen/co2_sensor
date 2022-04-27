#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stdint.h>
#include "driver/i2c.h"
#include <string.h>
#include "file_actions.h"
#include <time.h>
#include "tm1637.h"

#define V2_I2C_MASTER_SCL_IO 27 /*!< gpio number for I2C master clock */
#define V2_I2C_MASTER_SDA_IO 14

#define V3_I2C_MASTER_SCL_IO 5 /*!< gpio number for I2C master clock */
#define V3_I2C_MASTER_SDA_IO 4
/*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define DISPLAY_ADDRESS 0x70

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0          /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                /*!< I2C ack value */
#define NACK_VAL 0x1               /*!< I2C nack value */

static const char *TAG = "example";


static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = V2_I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = V2_I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    return i2c_param_config(I2C_MASTER_NUM, &conf);
}

void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = V2_I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = V2_I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    esp_err_t res = i2c_param_config(i2c_master_port, &conf);
    printf("Driver param setup : %d\n", res);
    res = i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    printf("Driver installed   : %d\n", res);
}

esp_err_t ma_write_byte(uint8_t address, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);
    //i2c_master_write_byte(cmd, address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 20 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL)
    {
        printf("ESP_I2C_WRITE ERROR : %d\n", ret);
        return ret;
    }
    return ESP_OK;
}

esp_err_t ma_write_byte_long(uint8_t address, uint8_t value, uint8_t value2)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);
    //i2c_master_write_byte(cmd, address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value2, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 20 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_FAIL)
    {
        printf("ESP_I2C_WRITE ERROR : %d\n", ret);
        return ret;
    }
    return ESP_OK;
}

void led_display_task(void *parameter)
{

    tm1637_led_t * lcd = tm1637_init(V3_I2C_MASTER_SCL_IO, V3_I2C_MASTER_SDA_IO);

    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
    tm1637_set_brightness(lcd, 2);
    while (true)
    {

        int co2 = read_co2();

        int N = co2;
        printf("CO2 %d \n",co2);
        int r = N % 10;
        tm1637_set_segment_number(lcd, 3, r, true);
        N = N / 10;
        r = N % 10;
        tm1637_set_segment_number(lcd, 2, r, true);
        N = N / 10;
        r = N % 10;
        tm1637_set_segment_number(lcd, 1, r, true);
        if (co2<1000){
            tm1637_set_segment_number(lcd, 0, 0, true);
        }else{
            N = N / 10;
            r = N % 10;
            tm1637_set_segment_number(lcd, 0, r, true);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}
