#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"

.bss

.global gpio_status
gpio_status: .long 0

.text

exit:
	halt

wakeup:
	// r0 should contain the gpio status - save it to our global variable
	MOVE r1, gpio_status
	ST r0, r1, 0
try_wakeup:
	// try to wakeup SoC, if not => halt
	STAGE_INC  1
	JUMPS exit, 10, GT

	READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
	AND r0, r0, 1
	JUMP try_wakeup, eq
	wake

	.global entry
entry:
	STAGE_RST

	READ_RTC_REG (RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
		
	AND R1, R0, 1<<4 // mask out the GPIO_NUM_34 pin
	JUMP wakeup, eq  // wakeup if it's low

	AND R1, R0, 1<<3 // mask out the GPIO_NUM_39 pin
	JUMP wakeup, eq  // wakeup if it's low

	AND R1, R0, 1<<5 // mask out the GPIO_NUM_35 pin
	JUMP wakeup, eq  // wakeup if it's low

	halt
