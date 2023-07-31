#pragma once

#include <Arduino.h>

char* str_copy(char* str);

char* str_repeat(char* str, int times);

char* str_span(char* src, int from, int to = -1);
