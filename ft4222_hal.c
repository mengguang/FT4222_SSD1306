//
// Created by mengguang on 2019/5/4.
//

#include "ft4222_hal.h"
#include "windows.h"
#include <stdio.h>
#include "ftd2xx.h"
#include "LibFT4222.h"
#include <stdint.h>

void HAL_Delay(uint32_t ms) {
    Sleep(ms);
}

uint32_t HAL_GetTick() {
    return (uint32_t) GetTickCount();
}