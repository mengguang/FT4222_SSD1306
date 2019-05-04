//
// Created by mengguang on 2019/5/4.
//

#ifndef FT4222_SSD1306_FT4222_HAL_H
#define FT4222_SSD1306_FT4222_HAL_H
#include "windows.h"
#include <stdio.h>
#include "ftd2xx.h"
#include "LibFT4222.h"
#include <stdint.h>

void HAL_Delay(uint32_t ms);

uint32_t HAL_GetTick();

#endif //FT4222_SSD1306_FT4222_HAL_H

