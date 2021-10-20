#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"

.bss

.text

.global entry
entry:
  // clear the hold bit on GPIO2
  WRITE_RTC_REG(RTC_IO_TOUCH_PAD2_REG, RTC_IO_TOUCH_PAD2_HOLD_S, 1, 0)
  // get the current value of the LED into bit 0 of R0
  READ_RTC_REG(RTC_GPIO_OUT_REG,RTC_GPIO_OUT_DATA_S+12,1)
  // if the LED is currentlt on, turn it off
  JUMPR turn_off, 1, EQ
  // led is currently off, turn it on
  WRITE_RTC_REG(RTC_GPIO_OUT_REG,RTC_GPIO_OUT_DATA_S+12,1,1)
  JUMP finish
turn_off:
  // led is currently on, turn it off
  WRITE_RTC_REG(RTC_GPIO_OUT_REG,RTC_GPIO_OUT_DATA_S+12,1,0)
finish:
  // set the hold bit on GPIO2
  WRITE_RTC_REG(RTC_IO_TOUCH_PAD2_REG, RTC_IO_TOUCH_PAD2_HOLD_S, 1, 1)
  // finish the program - we'll be woken up again to toggle the LED
  HALT
