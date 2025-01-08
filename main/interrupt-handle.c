#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_18
#define BUTTON_PIN GPIO_NUM_19

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR button_isr_handler(void* arg) {
    int pin = (int)arg;
    xQueueSendFromISR(gpio_evt_queue, &pin, NULL);
}

void button_task(void* arg) {
    int pin;
    static int led_state = 0;

    while (true) {
        if (xQueueReceive(gpio_evt_queue, &pin, portMAX_DELAY)) {
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);
            printf("Button pressed, LED state toggled to: %d\n", led_state);
        }
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

    gpio_evt_queue = xQueueCreate(10, sizeof(int));

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, (void*)BUTTON_PIN);
}
