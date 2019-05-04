cmake_minimum_required(VERSION 3.14)
project(FT4222_SSD1306 C)

set(CMAKE_C_STANDARD 99)
link_directories(lib)
add_executable(FT4222_SSD1306 main.c ssd1306.c ssd1306.h ssd1306_fonts.c ssd1306_fonts.h ssd1306_tests.c ssd1306_tests.h ft4222_hal.h ft4222_hal.c)
include_directories(lib)

target_link_libraries(FT4222_SSD1306 ftd2xx)
target_link_libraries(FT4222_SSD1306 libFT4222)


