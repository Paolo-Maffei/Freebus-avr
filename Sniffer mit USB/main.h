#include "inttypes.h"
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "avr/wdt.h"
#include "avr/interrupt.h"
#include "util/delay.h"
#include "stdlib.h"
#include "stdio.h"
#include "util/parity.h"

#include "cppstuff.h"
#include "task.h"
#include "timer.h"
#include "application.h"
#include "led.h"
#include "uart.h"
#include "message.h"
#include "eibreceive.h"
