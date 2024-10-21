#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#define WIFI_SSID "Airtel_thej_9819"
#define WIFI_PASS "air41550"
#define OTA_URL "http://Swathibpsanjay.github.io//FOTA.bin"


static const char *TAG = "FOTA";

//printf("LED ON");


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
                esp_wifi_connect();
                ESP_LOGI(TAG, "Retrying connection...");
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa((const ip4_addr_t *)&event->ip_info.ip));
        //      ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
        }
}
void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
                .sta = {
                                .ssid = WIFI_SSID,
                                    .password = WIFI_PASS,
                                    },
        };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
}
void ota_task(void *pvParameter)
{
        ESP_LOGI(TAG, "Starting OTA update...");
        esp_http_client_config_t config = {
                                .url = OTA_URL,

                                        .event_handler = NULL,
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);
        const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
        esp_err_t ret = esp_http_client_perform(client);
        if (ret == ESP_OK)
        {
                ESP_LOGI(TAG, "OTA done. Rebooting...");
                esp_ota_set_boot_partition(update_partition);
                esp_restart();
        }
        else
        {
                ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(ret));
        }
        esp_http_client_cleanup(client);
}
void app_main(void)
{
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        ESP_LOGI(TAG, "ESP32 FOTA Example");
        wifi_init_sta();
        xTaskCreate(ota_task,"ota_task",8192, NULL, 5, NULL);
}

