#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "tcpip_adapter.h"

#define ServerIPAddress "91.215.156.152"
#define ServerDomainName "realfm.live24.gr"
#define ServerPath "\realfm"
#define ServerPortNumber 80

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAG = "JSB";
static int s_retry_num = 0;

static uint8_t WiFiConnected = 0;

static void TCP_Go()
{
  char addr_str[128];
  int addr_family;
  int ip_protocol;
  int err;

  do
  {
    struct sockaddr_in ServerDescriptor;

    // ServerDescriptor.sin_addr.s_addr = inet_addr(ServerIPAddress);

    struct hostent *Host;
    Host = gethostbyname(ServerDomainName);
    ServerDescriptor.sin_addr.s_addr = *(long *)(Host->h_addr_list[0]);

    ServerDescriptor.sin_family = AF_INET;
    ServerDescriptor.sin_port = htons(ServerPortNumber);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(ServerDescriptor.sin_addr, addr_str, sizeof(addr_str) - 1);

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
      ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(TAG, "Socket created");

    ESP_LOGI(TAG, "Connecting to %s:%d", ServerIPAddress, ServerPortNumber);
    err = connect(sock, (struct sockaddr *)&ServerDescriptor, sizeof(ServerDescriptor));
    if (err != 0)
    {
      ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
      break;
    }
    ESP_LOGI(TAG, "Successfully connected");

    // Request stream:
    ESP_LOGI(TAG, "Requesting stream");
    char RequestToServer[256];
    sprintf(RequestToServer, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", ServerPath, ServerDomainName);
    err = send(sock, RequestToServer, strlen(RequestToServer), 0);
    if (err < 0)
    {
      ESP_LOGE(TAG, "Error occurred during sending request. errno=%d", errno);
      break;
    }
    ESP_LOGI(TAG, "Stream request succeeded");

    while (1)
    {
      char rx_buffer[128];
      // int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
      int len;
      do
      {
        len = recv(sock, rx_buffer, 32, 0);
        if (len < 0)
          ESP_LOGE(TAG, "recv failed. errno=%d", errno);
      } while (len < 0);

      rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
      ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
      ESP_LOGI(TAG, "Received: %s", rx_buffer);

      // vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    if (sock != -1)
    {
      ESP_LOGE(TAG, "Closing socket");
      shutdown(sock, 0);
      close(sock);
    }
  } while(0);

  vTaskDelete(NULL);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT)
  {
    if (event_id == WIFI_EVENT_STA_START)
    {
      esp_wifi_connect();
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
      if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
      {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        s_retry_num++;
        ESP_LOGI(TAG, "Retrying to connect to access point");
      }
      else
        ESP_LOGI(TAG, "Failed to connect to access point");
    }
  }
  else if (event_base == IP_EVENT)
  {
    if (event_id == IP_EVENT_STA_GOT_IP)
    {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
      ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
      s_retry_num = 0;
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
      WiFiConnected = 1;
    }
  }
}

void WiFi_Connect(void)
{
  WiFiConnected = 0;

  s_wifi_event_group = xEventGroupCreate();

  tcpip_adapter_init();

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  wifi_config_t wifi_config =
      {
          .sta =
              {
                  .ssid = EXAMPLE_ESP_WIFI_SSID,
                  .password = EXAMPLE_ESP_WIFI_PASS
              },
      };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Connecting to WiFi.");

  while (!WiFiConnected)
    vTaskDelay(500 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "Connected to WiFi.");
}

void Go(void *pvParameters)
{
  WiFi_Connect();
  TCP_Go();
}

void app_main(void)
{
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  xTaskCreate(Go, "Go", 4096, NULL, 5, NULL);
}
