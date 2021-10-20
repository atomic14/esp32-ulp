#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <esp32/ulp.h>
#include "ulp_main.h"

const gpio_num_t BUTTON_PIN = GPIO_NUM_0;
const gpio_num_t LED_PIN = GPIO_NUM_2;

extern "C"
{
  void app_main();
}

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

void app_main()
{
  // turn on the LED
  // gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  // gpio_set_level(LED_PIN, 1);

  // did we wake up from the ULP?
  auto wake_cause = esp_sleep_get_wakeup_cause();
  if (wake_cause != ESP_SLEEP_WAKEUP_ULP)
  {
    // we were woken up for some other reason or it's a fresh boot
    // setup the ULP program
    ESP_LOGI("main", "Loading the ULP binary");
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);
  }
  else
  {
    // we were woken by the ULP
    ESP_LOGI("main", "Woken up by the ULP");
  }
  gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
  // wait for the button to be pushed
  while (gpio_get_level(BUTTON_PIN) == 1)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  // setup the RCP pin
  rtc_gpio_init(GPIO_NUM_2);
  rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY);
  // goto sleep
  ESP_LOGI("main", "Going to sleep");
  esp_err_t err = esp_sleep_enable_ulp_wakeup();
  ESP_ERROR_CHECK(err);
  err = ulp_set_wakeup_period(0, 500000); // 0.5 seconds
  ESP_ERROR_CHECK(err);
  err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
  ESP_ERROR_CHECK(err);
  esp_deep_sleep_start();
}