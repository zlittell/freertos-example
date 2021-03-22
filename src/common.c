#include "common.h"

static uint32_t SystemCoreClock = 0;

uint32_t GetSystemCoreClock(void)
{
    return SystemCoreClock;
}

void SetSystemCoreClock(uint32_t coreSpeed)
{
    SystemCoreClock = coreSpeed;
}