#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/dac.h"

uint8_t num = 1;
enum {Increase, Decrease} direction = Increase;

void MyTimer(void) {
	switch (direction)
	{
	case Increase:
		dac_output_voltage(DAC_CHANNEL_1, num++);
		if (num >= 255) direction = Decrease;
		break;
	case Decrease:
		dac_output_voltage(DAC_CHANNEL_1, num--);
		if (num <= 0) direction = Increase;
		break;
	default:
		break;
	}
}

void app_main(void) {

	/**************************************************/
	/* Configure DAC (Digital to Analog Converter)    */

	/* DAC_CHANNEL_1 = GPIO25 (IO25); */
	dac_output_enable(DAC_CHANNEL_1);

	/* Set Value */
	dac_output_voltage(DAC_CHANNEL_1, 0);

	float timeUs = 10000;

	/**************************************************/
	/* Configure Timer                                */
    const esp_timer_create_args_t esp_timer_create_args = {
        .callback = MyTimer,
        .name = "MyTimer"
		};
    esp_timer_handle_t esp_timer_handle;
    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);

	/* Start timer  */
    esp_timer_start_periodic(esp_timer_handle, timeUs);

	/* Display timer information */
	esp_timer_dump(stdout);

	vTaskDelay(pdMS_TO_TICKS(15000));

	timeUs = 1000;

	/* Stop Timer */
	esp_timer_stop(esp_timer_handle);

	/* Re-Start timer  */
    esp_timer_start_periodic(esp_timer_handle, timeUs);

	/* Delete Timer */
	esp_timer_delete(esp_timer_handle);

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
