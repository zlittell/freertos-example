#include "sam.h"
#include "common.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

static const short stacksize_BlinkLED = configMINIMAL_STACK_SIZE;
static const uint8_t priority_BlinkLED = 2;
//static TaskHandle_T handle_BlinkLED;
static const short stacksize_RotateCounter = configMINIMAL_STACK_SIZE;
static const uint8_t priority_RotateCounter = 3;

/**
	@brief Configure System Clocks
	@details Configures clocks for a SAMD11
*/
void init_Clocks(void)
{
	// _pm_init
	PM_REGS->PM_CPUSEL= PM_CPUSEL_CPUDIV_DIV1;	// CPU Clock Divide by 1
	
	// _sysctrl_init_sources
	SYSCTRL_REGS->SYSCTRL_OSC8M = 	((SYSCTRL_REGS->SYSCTRL_OSC8M & SYSCTRL_OSC8M_FRANGE_Msk) | 
									(SYSCTRL_REGS->SYSCTRL_OSC8M & SYSCTRL_OSC8M_CALIB_Msk) | 
									SYSCTRL_OSC8M_PRESC_0 | SYSCTRL_OSC8M_ENABLE(1));
	while((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_OSC8MRDY(1))==0);
	SYSCTRL_REGS->SYSCTRL_OSC8M |= SYSCTRL_OSC8M_ONDEMAND(1);
	
	// _sysctrl init referenced
	SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_ENABLE(1);
	while((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY(1)) == 0);
	SYSCTRL_REGS->SYSCTRL_DFLLMUL = (SYSCTRL_DFLLMUL_CSTEP(1) | SYSCTRL_DFLLMUL_FSTEP(1) | SYSCTRL_DFLLMUL_MUL(48000));
	SYSCTRL_REGS->SYSCTRL_DFLLCTRL = (SYSCTRL_DFLLCTRL_CCDIS(1) | SYSCTRL_DFLLCTRL_USBCRM(1) | SYSCTRL_DFLLCTRL_MODE(1) | SYSCTRL_DFLLCTRL_ENABLE(1));
	while((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY(1)) == 0);
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY(1));
	
	// _gclk_init_generators last init
	GCLK_REGS->GCLK_GENDIV = (GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(0));
	GCLK_REGS->GCLK_GENCTRL = (GCLK_GENCTRL_GENEN(1) | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(0));
	GCLK_REGS->GCLK_GENDIV = (GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(1));
	GCLK_REGS->GCLK_GENCTRL = (GCLK_GENCTRL_GENEN(1) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(1));
	GCLK_REGS->GCLK_GENDIV = (GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(3));
	GCLK_REGS->GCLK_GENCTRL = (GCLK_GENCTRL_GENEN(1) | GCLK_GENCTRL_SRC_OSC32K | GCLK_GENCTRL_ID(3));

	SetSystemCoreClock(48000000);
}

void init_IO(void)
{
	// OnboardLED - PA16
	PORT_REGS->GROUP[0].PORT_DIRSET = PORT_DIRSET_DIRSET(1u<<16);
	PORT_REGS->GROUP[0].PORT_OUTCLR = PORT_OUTCLR_OUTCLR(1u<<16);
}

void vBlinkLED(void *pvParameters)
{
	for (;;)
	{
		PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_DIRSET_DIRSET(1u<<16);
	}

	vTaskDelete(NULL);
}

void vRotateCounter(void *pvParameters)
{
	uint8_t counter = 0;

	for (;;)
	{
		if (counter < 255)
		{
			counter++;
		}
		else
		{
			counter = 0;
		}
	}
}

int main(void)
{
	init_Clocks();
	init_IO();

	xTaskCreate(vBlinkLED,
				"LED Blinking Task",
				stacksize_BlinkLED,
				NULL,
				priority_BlinkLED,
				NULL);

	for(;;){}
}