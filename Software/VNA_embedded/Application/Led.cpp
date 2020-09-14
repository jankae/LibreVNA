#include "Led.hpp"

#include "stm.hpp"
#include "FreeRTOS.h"
#include "task.h"

#if HW_REVISION == 'B'

#define LED_TASK_STACK		128

extern TIM_HandleTypeDef htim2;

enum class Mode {
	Off,
	On,
	Blink,
	Pulsating,
	Error,
};

static Mode mode;
static uint8_t led_statecnt;
static int8_t led_ncnt;
static xTaskHandle task;
static StaticTask_t xTask;
static StackType_t xStack[LED_TASK_STACK];
static uint8_t err_cnt;

static void led_set_percentage(uint8_t val) {
	uint16_t compare = val * val / 100;
	TIM2->CCR1 = compare;
}

static void led_task(void* unused) {
	UNUSED(unused);
	while (1) {
		if (led_statecnt < 199) {
			led_statecnt++;
		} else {
			led_statecnt = 0;
		}
		switch (mode) {
		case Mode::Off:
			led_set_percentage(0);
			vTaskSuspend(NULL);
			break;
		case Mode::On:
			led_set_percentage(100);
			vTaskSuspend(NULL);
			break;
		case Mode::Error:
			if (led_statecnt == 0) {
				err_cnt++;
				if (err_cnt > led_ncnt) {
					// reached number of blinks + 1
					err_cnt = 0;
				}
			}
			if (err_cnt >= led_ncnt) {
				break;
			}
			// fall through
			/* no break */
		case Mode::Blink:
			if(led_statecnt < 100) {
				led_set_percentage(100);
			} else {
				led_set_percentage(0);
			}
			break;
		case Mode::Pulsating:
			if (led_statecnt < 100) {
				led_set_percentage(led_statecnt);
			} else {
				led_set_percentage(200 - led_statecnt);
			}
			break;
		}
		vTaskDelay(5);
	}
}
#endif

void LED::Init() {
#if HW_REVISION == 'B'
	led_ncnt = 0;
	mode = Mode::Off;
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	task = xTaskCreateStatic(led_task, "LED",
	LED_TASK_STACK, NULL, 6, xStack, &xTask);
	vTaskSuspend(task);
#endif
}

void LED::Pulsating() {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(task);
	mode = Mode::Pulsating;
	vTaskResume(task);
#endif
}

void LED::Off() {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(task);
	mode = Mode::Off;
	vTaskResume(task);
#endif
}

void LED::Error(uint8_t code) {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(task);
	mode = Mode::Error;
	led_statecnt = 0;
	err_cnt = 0;
	led_ncnt = code;
	vTaskResume(task);
#endif
}
