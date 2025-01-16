#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LED_PIN GPIO_NUM_18
#define BUTTON_PIN GPIO_NUM_19

static esp_timer_handle_t debounce_timer;
static volatile bool button_pressed = false;
static volatile bool led_state = false;

void IRAM_ATTR debounce_timer_callback(void *arg) {
    if (gpio_get_level(BUTTON_PIN) == 0) {
        led_state = !led_state;
        gpio_set_level(LED_PIN, led_state);
    }
    button_pressed = false;
}

void gpio_isr_handler(void *arg) {
    if (!button_pressed) {
        button_pressed = true;
        esp_timer_start_once(debounce_timer, 50000);
    }
}

void app_main(void) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    esp_timer_create_args_t timer_args = {
        .callback = debounce_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "debounce_timer"};
    esp_timer_create(&timer_args, &debounce_timer);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, NULL);

}
