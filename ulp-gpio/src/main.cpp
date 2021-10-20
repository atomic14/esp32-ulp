#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <esp32/ulp.h>

extern "C"
{
  void app_main();
}

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

uint8_t *frame_buffer = nullptr;

void app_main()
{
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
    ESP_LOGI("main", "####### Woken up by the ULP");
    // which button woke us up?
    uint16_t rtc_pin = ulp_gpio_status & UINT16_MAX;
    ESP_LOGI("main", "rtc_pin %x woke us up", rtc_pin);
    if ((rtc_pin & (1 << rtc_io_number_get(GPIO_NUM_34))) == 0)
    {
      ESP_LOGI("main", "****** ULP - GPIO_NUM_34");
    }
    else if ((rtc_pin & (1 << rtc_io_number_get(GPIO_NUM_35))) == 0)
    {
      ESP_LOGI("main", "****** ULP - GPIO_NUM_35");
    }
    else if ((rtc_pin & (1 << rtc_io_number_get(GPIO_NUM_39))) == 0)
    {
      ESP_LOGI("main", "****** ULP - GPIO_NUM_39");
    }
  }
  // wait for 5 seconds
  vTaskDelay(pdMS_TO_TICKS(250));
  ESP_LOGI("main", "Going to sleep...3");
  vTaskDelay(pdMS_TO_TICKS(250));
  ESP_LOGI("main", "Going to sleep...2");
  vTaskDelay(pdMS_TO_TICKS(250));
  ESP_LOGI("main", "Going to sleep...1");
  vTaskDelay(pdMS_TO_TICKS(250));
  // goto sleep
  ESP_LOGI("main", "Going to sleep");
  rtc_gpio_init(GPIO_NUM_34);
  rtc_gpio_set_direction(GPIO_NUM_34, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_init(GPIO_NUM_35);
  rtc_gpio_set_direction(GPIO_NUM_35, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_init(GPIO_NUM_39);
  rtc_gpio_set_direction(GPIO_NUM_39, RTC_GPIO_MODE_INPUT_ONLY);
  // enable ulp wakeup
  esp_err_t err = esp_sleep_enable_ulp_wakeup();
  ESP_ERROR_CHECK(err);
  err = ulp_set_wakeup_period(0, 100000); // 100ms
  ESP_ERROR_CHECK(err);
  err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
  ESP_ERROR_CHECK(err);
  esp_deep_sleep_start();
}