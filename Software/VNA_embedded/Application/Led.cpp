#include "Led.hpp"

#include "stm.hpp"
#include "FreeRTOS.h"
#include "task.h"

#if HW_REVISION == 'B'

#define LED_STATUS_TASK_STACK_SIZE_WORDS	128

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

static StackType_t LedStatusStack[LED_STATUS_TASK_STACK_SIZE_WORDS];
static StaticTask_t LedStatusCB;
static xTaskHandle LedStatusHandle = NULL;

static uint8_t err_cnt;

static void led_set_percentage(uint8_t val) {
	uint16_t compare = val * val / 100;
	TIM2->CCR1 = compare;
}

static void LedStatus(void * const argument) {
	UNUSED(argument);
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

	LedStatusHandle = xTaskCreateStatic(LedStatus, "LedStatusTask", LED_STATUS_TASK_STACK_SIZE_WORDS,
										NULL, 6, LedStatusStack, &LedStatusCB);

	vTaskSuspend(LedStatusHandle);
#endif
}

void LED::Pulsating() {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(LedStatusHandle);
	mode = Mode::Pulsating;
	vTaskResume(LedStatusHandle);
#endif
}

void LED::Off() {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(LedStatusHandle);
	mode = Mode::Off;
	vTaskResume(LedStatusHandle);
#endif
}

void LED::Error(uint8_t code) {
#if HW_REVISION == 'B'
	if(led_ncnt) {
		return;
	}
	vTaskSuspend(LedStatusHandle);
	mode = Mode::Error;
	led_statecnt = 0;
	err_cnt = 0;
	led_ncnt = code;
	vTaskResume(LedStatusHandle);
#endif
}
