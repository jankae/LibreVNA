#include "Led.hpp"

#include "stm.hpp"
#include "FreeRTOS.h"
#include "task.h"


#define LED_TASK_STACK		4096

extern TIM_HandleTypeDef htim2;

enum class Mode {
	Off,
	On,
	Blink,
	Pulsating,
	Error,
};

static Mode mode[4];
static uint8_t led_statecnt[4];
static int8_t led_ncnt[4];
static xTaskHandle task;
static StaticTask_t xTask;
static StackType_t xStack[LED_TASK_STACK];
static uint8_t err_cnt[4];

static void led_set_percentage(uint8_t led, uint8_t val) {
	uint16_t compare = val * val / 100;
	switch(led) {
	case 0: TIM2->CCR1 = compare; break;
	case 1: TIM2->CCR2 = compare; break;
	case 2: TIM2->CCR3 = compare; break;
	case 3: TIM2->CCR4 = compare; break;
	}
}

static void led_task(void* unused) {
	UNUSED(unused);
	while (1) {
		for(uint8_t led = 0;led < 4;led++) {
			if (led_statecnt[led] < 199) {
				led_statecnt[led]++;
			} else {
				led_statecnt[led] = 0;
			}
			switch (mode[led]) {
			case Mode::Off:
				led_set_percentage(led, 0);
				break;
			case Mode::On:
				led_set_percentage(led, 100);
				break;
			case Mode::Error:
				if (led_statecnt[led] == 0) {
					err_cnt[led]++;
					if (err_cnt[led] > led_ncnt[led]) {
						// reached number of blinks + 1
						err_cnt[led] = 0;
					}
				}
				if (err_cnt[led] >= led_ncnt[led]) {
					break;
				}
				// fall through
				/* no break */
			case Mode::Blink:
				if(led_statecnt[led] < 100) {
					led_set_percentage(led, 100);
				} else {
					led_set_percentage(led, 0);
				}
				break;
			case Mode::Pulsating:
				if (led_statecnt[led] < 100) {
					led_set_percentage(led, led_statecnt[led]);
				} else {
					led_set_percentage(led, 200 - led_statecnt[led]);
				}
				break;
			}
		}
		vTaskDelay(5);
	}
}

void LED::Init() {
	led_ncnt[0] = 0;
	led_ncnt[1] = 0;
	led_ncnt[2] = 0;
	led_ncnt[3] = 0;
	mode[0] = Mode::Off;
	mode[1] = Mode::Off;
	mode[2] = Mode::Off;
	mode[3] = Mode::Off;
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

	task = xTaskCreateStatic(led_task, "LED",
	LED_TASK_STACK, NULL, 6, xStack, &xTask);
	vTaskSuspend(task);
}

void LED::Pulsating(uint8_t led) {
	if(led_ncnt[led]) {
		return;
	}
	vTaskSuspend(task);
	mode[led] = Mode::Pulsating;
	vTaskResume(task);
}

void LED::Off(uint8_t led) {
	if(led_ncnt[led]) {
		return;
	}
	vTaskSuspend(task);
	mode[led] = Mode::Off;
	vTaskResume(task);
}

void LED::On(uint8_t led) {
	if(led_ncnt[led]) {
		return;
	}
	vTaskSuspend(task);
	mode[led] = Mode::On;
	vTaskResume(task);
}

void LED::Error(uint8_t led, uint8_t code) {
	if(led_ncnt[led]) {
		return;
	}
	vTaskSuspend(task);
	mode[led] = Mode::Error;
	led_statecnt[led] = 0;
	err_cnt[led] = 0;
	led_ncnt[led] = code;
	vTaskResume(task);
}
