// SPDX-FileCopyrightText: 2026 Heartland Data inc.
// SPDX-License-Identifier: Apache-2.0

/*==============================================================================*/
/*  Copyright (C) 2026, Heartland Data inc. All Rights Reserved.                */
/*                                                                              */
/*  Title  :   GPIO 2bit Driver                                                 */
/*  EventID:   Via ther driver                                                  */
/*  AppVer :   DT+ Ver1.00`                                                     */
/*  FileID :   13b                                                              */
/*  Version:   1.0                                                              */
/*  Author :   HLDC                                                             */
/*==============================================================================*/

/*==============================================================================*/
/*  Please customize the code for your environment.                             */
/*==============================================================================*/

/*==============================================================================*/
/*  Desc:   Header for Port Control                                             */
/*==============================================================================*/
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==============================================================================*/
/*  Macro:  DT_UINT                                                             */
/*  Desc:   Please change Test Point argument type for DT+ Project setting.     */
/*==============================================================================*/
#define DT_UINT unsigned int

/*==============================================================================*/
/*  Macro:  DT_INLINE                                                           */
/*  Desc:   Please use "static" instead of "inline" if "inline" cannot be used. */
/*==============================================================================*/
#define DT_INLINE static inline
/* #define	DT_INLINE	static */

/*==============================================================================*/
/*  Macro:  DT_ADD_EVENT_INFO                                                   */
/*  Desc:   Please set 1 when you add Event Value to Test Point.                */
/*==============================================================================*/
#define DT_ADD_EVENT_INFO 0

/*==============================================================================*/
/*  GPIO Pin Definitions for ESP32                                              */
/*==============================================================================*/
#define DT_GPIO_CLK GPIO_NUM_4	 /* DT+_1: IO4 - CLK */
#define DT_GPIO_CS GPIO_NUM_18	 /* DT+_2: IO18 - CS */
#define DT_GPIO_DAT0 GPIO_NUM_19 /* DT+_3: IO19 - DAT0 */
#define DT_GPIO_DAT1 GPIO_NUM_23 /* DT+_4: IO23 - DAT1 */

static uint32_t clk_level;

/*==============================================================================*/
/*  Func:   _TP_BusPortInit                                                     */
/*  Desc:   Please describe the code to initializes ports.                      */
/*==============================================================================*/
static void _TP_BusPortInit(void)
{
	/* Initialize GPIO pins for ESP32 */
	gpio_config_t io_conf = {};

	/* Configure CLK pin (GPIO4) */
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = ((1ULL << DT_GPIO_CLK) |
	                        (1ULL << DT_GPIO_CS) |
	                        (1ULL << DT_GPIO_DAT0) |
	                        (1ULL << DT_GPIO_DAT1));
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);

	/* Set initial values - all pins HIGH */
	clk_level = 1;
	gpio_set_level(DT_GPIO_CLK, clk_level);
	gpio_set_level(DT_GPIO_CS, 1);
	gpio_set_level(DT_GPIO_DAT0, 1);
	gpio_set_level(DT_GPIO_DAT1, 1);
}

/*==============================================================================*/
/*  Func:   portReverseCLK                                                      */
/*  Desc:   Please describe the code to reverse CLK Port.                       */
/*==============================================================================*/
DT_INLINE void portReverseCLK(void)
{
	/* Toggle CLK pin */
	clk_level = !clk_level;
	gpio_set_level(DT_GPIO_CLK, clk_level);
}

/*==============================================================================*/
/*  Func:   portSetCLK                                                          */
/*  Desc:   Please describe the code to set the value to CLK Port.              */
/*==============================================================================*/
DT_INLINE void portSetCLK(DT_UINT dat)
{
	/* Set CLK pin level */
	gpio_set_level(DT_GPIO_CLK, dat ? 1 : 0);
}

/*==============================================================================*/
/*  Func:   portSetCS                                                           */
/*  Desc:   Please describe the code to set the value to CS Port.               */
/*==============================================================================*/
DT_INLINE void portSetCS(DT_UINT dat)
{
	/* Set CS pin level */
	gpio_set_level(DT_GPIO_CS, dat ? 1 : 0);
}

/*==============================================================================*/
/*  Func:   portSetDATA                                                         */
/*  Desc:   Please describe the code to set the value to DATA Port              */
/*==============================================================================*/
DT_INLINE void portSetDATA(DT_UINT dat)
{
	/* Set DAT0 and DAT1 pins based on 2-bit data */
	gpio_set_level(DT_GPIO_DAT0, (dat & 0x01) ? 1 : 0); /* bit 0 */
	gpio_set_level(DT_GPIO_DAT1, (dat & 0x02) ? 1 : 0); /* bit 1 */
}

/*==============================================================================*/
/*  Global variable for critical section                                        */
/*==============================================================================*/
static portMUX_TYPE dt_spinlock = portMUX_INITIALIZER_UNLOCKED;

/*==============================================================================*/
/*  Func:   enterCritical                                                       */
/*  Desc:   Please describe the code to enter Critical Section.                 */
/*==============================================================================*/
DT_INLINE void enterCritical(void)
{
	/* Enter critical section */
	taskENTER_CRITICAL(&dt_spinlock);
}

/*==============================================================================*/
/*  Func:   exitCritical                                                        */
/*  Desc:   Please describe the code to exit Critical Section.                  */
/*==============================================================================*/
DT_INLINE void exitCritical(void)
{
	/* Exit critical section */
	taskEXIT_CRITICAL(&dt_spinlock);
}

/*==============================================================================*/
/*  Func:   addEventInfo                                                        */
/*  Desc:   please describe the code to get target event                        */
/*==============================================================================*/
#if DT_ADD_EVENT_INFO
DT_INLINE DT_UINT addEventInfo(void)
{
	int event_id = 0;

	/* describe code to get target eventinformation */
	/* ex. uITron4.0 API	*/
	// ID task_id;
	// get_tid( &p_task_id );
	// event_id = (DT_UINT)task_id;
	return event_id;
}
#endif

/*==============================================================================*/
/*  Don't change the code from here as possible.                                */
/*==============================================================================*/

/*==============================================================================*/
/*  Desc:   Test Point Parametaer                                               */
/*==============================================================================*/
#define DT_VARIABLE_BIT 0x02
#define DT_TARGET_TIME_BIT 0x04
#define DT_EVTTRG_BIT 0x08
#define DT_VARIABLE_FAST_BIT 0x01
#define DT_EVTTRG_FAST_BIT 0x02
#define DT_VARIABLE_WRITE_BIT 0x01

/*==============================================================================*/
/*  Func:   portInit                                                            */
/*  Desc:   Initialize Port Function                                            */
/*==============================================================================*/
static int init = 0;
DT_INLINE void portInit(void)
{
	if (init == 0)
	{
		_TP_BusPortInit();
		init = 1;
	}
}

/*==============================================================================*/
/*  Func:   _TP_Bus2BitOutDrv                                                   */
/*  Desc:   2bit Data Output Function                                           */
/*==============================================================================*/
DT_INLINE void _TP_Bus2BitOutDrv(DT_UINT dat)
{
	portSetDATA(dat);
	portReverseCLK();
}

/*==============================================================================*/
/*  Func: _TP_BusOutDrv                                                         */
/*  Desc: Test Point Output Function                                            */
/*==============================================================================*/
DT_INLINE void _TP_BusOutDrv(DT_UINT addr, DT_UINT dat)
{
	portSetCS(0);
	_TP_Bus2BitOutDrv(dat >> 14);
	_TP_Bus2BitOutDrv(dat >> 12);
	_TP_Bus2BitOutDrv(dat >> 10);
	_TP_Bus2BitOutDrv(dat >> 8);
	_TP_Bus2BitOutDrv(dat >> 6);
	_TP_Bus2BitOutDrv(dat >> 4);
	_TP_Bus2BitOutDrv(dat >> 2);
	_TP_Bus2BitOutDrv(dat);
	addr &= 0xfffff;
	if (addr >= 0x40000)
		_TP_Bus2BitOutDrv(addr >> 18);
	if (addr >= 0x10000)
		_TP_Bus2BitOutDrv(addr >> 16);
	if (addr >= 0x4000)
		_TP_Bus2BitOutDrv(addr >> 14);
	if (addr >= 0x1000)
		_TP_Bus2BitOutDrv(addr >> 12);
	if (addr >= 0x0400)
		_TP_Bus2BitOutDrv(addr >> 10);
	if (addr >= 0x0100)
		_TP_Bus2BitOutDrv(addr >> 8);
	if (addr >= 0x0040)
		_TP_Bus2BitOutDrv(addr >> 6);
	if (addr >= 0x0010)
		_TP_Bus2BitOutDrv(addr >> 4);
	if (addr >= 0x0004)
		_TP_Bus2BitOutDrv(addr >> 2);
	_TP_Bus2BitOutDrv(addr);
	portSetCS(1);
}

/*==============================================================================*/
/*  Func:   _TP_BusOutByteDrv                                                   */
/*  Desc:   Byte Data Output Function                                           */
/*==============================================================================*/
DT_INLINE void _TP_BusOutByteDrv(DT_UINT dat)
{
	_TP_Bus2BitOutDrv(dat >> 6);
	_TP_Bus2BitOutDrv(dat >> 4);
	_TP_Bus2BitOutDrv(dat >> 2);
	_TP_Bus2BitOutDrv(dat);
}

/*==============================================================================*/
/*  Func:   _TP_MemoryOutDrv                                                    */
/*  Desc:   Value Output Function                                               */
/*==============================================================================*/
DT_INLINE void _TP_MemoryOutDrv(unsigned char *p, DT_UINT size)
{
	if (size >= 256)
		size = 256;
	_TP_BusOutByteDrv(size);
	for (; size != 0; --size, ++p)
	{
		_TP_BusOutByteDrv(*p);
	}
}

/*==============================================================================*/
/*  Func: _TP_BusOut                                                            */
/*  Desc: Called by Test Point                                                  */
/*==============================================================================*/
void _TP_BusOut(DT_UINT addr, DT_UINT dat)
{

#if DT_ADD_EVENT_INFO
	unsigned int event_id = 0;
#endif

	portInit();
	enterCritical();

#if DT_ADD_EVENT_INFO
	addr = addr | DT_EVTTRG_BIT;
#endif

	_TP_BusOutDrv(addr, dat);

#if DT_ADD_EVENT_INFO
	portSetCS(0);

	event_id = addEventInfo();
	_TP_BusOutByteDrv(event_id);
	_TP_BusOutByteDrv(event_id >> 8);
	_TP_BusOutByteDrv(event_id >> 16);
	_TP_BusOutByteDrv(event_id >> 24);

	portSetCS(1);
#endif
	exitCritical();
}

/*==============================================================================*/
/*  Func:   _TP_MemoryOutput                                                    */
/*  Desc:   Called by Variable Test Point                                       */
/*==============================================================================*/
void _TP_MemoryOutput(DT_UINT addr, DT_UINT dat, void *value, DT_UINT size)
{

#if DT_ADD_EVENT_INFO
	int event_id = 0;
#endif

	portInit();
	enterCritical();

#if DT_ADD_EVENT_INFO
	addr = addr | DT_EVTTRG_BIT;
#endif

	_TP_BusOutDrv(addr | DT_VARIABLE_BIT, dat);

	/* 2nd data output */
	portSetCS(0);

#if DT_ADD_EVENT_INFO
	event_id = addEventInfo();
	_TP_BusOutByteDrv(event_id);
	_TP_BusOutByteDrv(event_id >> 8);
	_TP_BusOutByteDrv(event_id >> 16);
	_TP_BusOutByteDrv(event_id >> 24);
#endif

	/* output value */
	_TP_MemoryOutDrv((unsigned char *)value, size);

	portSetCS(1);
	exitCritical();
}
