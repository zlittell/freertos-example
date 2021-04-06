#include "sam.h"
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"

static const short stacksize_BlinkLED = configMINIMAL_STACK_SIZE;
static const uint8_t priority_BlinkLED = 2;
//static TaskHandle_T handle_BlinkLED;
static const short stacksize_RotateCounter = configMINIMAL_STACK_SIZE;
static const uint8_t priority_RotateCounter = 3;

void vBlinkLED(void *pvParameters);

/**
	@brief Configure System Clocks
	@details Configures clocks for a SAMD11
*/
void init_Clocks(void)
{
	NVMCTRL_REGS->NVMCTRL_CTRLB |= NVMCTRL_CTRLB_RWS(1);

	// Enable internal 32.768K oscillator
	SYSCTRL_REGS->SYSCTRL_OSC32K = 
						SYSCTRL_OSC32K_STARTUP(SYSCTRL_OSC32K_STARTUP_CYCLE10_Val) |
						SYSCTRL_OSC32K_EN32K(1) |
						SYSCTRL_OSC32K_CALIB(((*((uint32_t *)(OTP4_ADDR+1)))&(0x1FC0))>>6);
	SYSCTRL_REGS->SYSCTRL_OSC32K |= SYSCTRL_OSC32K_ENABLE(1);	// Must be seperate call

	// Wait for OSC32 to start
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_OSC32KRDY(1)));

	// Route internal 32.768K oscillator to CLOCK GEN 1
	GCLK_REGS->GCLK_GENDIV = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);
	GCLK_REGS->GCLK_GENCTRL = 
							GCLK_GENCTRL_ID(1) |
							GCLK_GENCTRL_SRC_OSC32K |
							GCLK_GENCTRL_IDC(1) |
							GCLK_GENCTRL_GENEN(1);

	// Wait for write sync
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY(1));

	//
	GCLK_REGS->GCLK_CLKCTRL = 
							GCLK_CLKCTRL_ID_DFLL48 | 
							GCLK_CLKCTRL_GEN_GCLK1 |
							GCLK_CLKCTRL_CLKEN(1);

	// Set up 48MHZ
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY(1)));
	SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_ENABLE(1);	// ERRATA 1.2.1
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY(1)));

	// Multiplier and lock steps
	SYSCTRL_REGS->SYSCTRL_DFLLMUL = 
								SYSCTRL_DFLLMUL_MUL(1465) |		// 37768*1465 ~= 48MHz
								SYSCTRL_DFLLMUL_FSTEP(511) |	// Half of Max 1023
								SYSCTRL_DFLLMUL_CSTEP(31);		// Half of Max 63
	while(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY(1)));

	// Load coarse calibration for faster lock
	uint32_t CoarseCalRead = ((*((uint32_t *)(OTP4_ADDR+1))) & 0xFC000000) >> 26;
	CoarseCalRead = ((CoarseCalRead == 0x3F) ? 0x1F : CoarseCalRead);
	SYSCTRL_REGS->SYSCTRL_DFLLVAL =  SYSCTRL_DFLLVAL_COARSE(CoarseCalRead);

	SYSCTRL_REGS->SYSCTRL_DFLLCTRL = 
							SYSCTRL_DFLLCTRL_MODE(1) |	// CLOSED LOOP
							SYSCTRL_DFLLCTRL_WAITLOCK(1) |	// Wait for lock before output
							SYSCTRL_DFLLCTRL_ENABLE(1);

	// Wait for lock 
	while ((!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLLCKC(1))) | 
			(!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLLCKF(1))));
	
	// Route 48MHz to GCLK0 for CPU
	GCLK_REGS->GCLK_GENCTRL = 
						GCLK_GENCTRL_ID(0) |
						GCLK_GENCTRL_SRC_DFLL48M |
						GCLK_GENCTRL_IDC(1) |
						GCLK_GENCTRL_GENEN(1);

	// Wait for write sync
	while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY(1));

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
	(void) pvParameters;
	for (;;)
	{
		PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_OUTTGL_OUTTGL(1u<<16);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void vRotateCounter(void *pvParameters)
{
	(void) pvParameters;
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

	vTaskStartScheduler();
	for(;;){}
}