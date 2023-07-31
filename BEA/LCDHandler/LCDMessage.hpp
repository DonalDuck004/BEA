#pragma once

#define PLAY_FOR_TICKS_DISABLED -1
#include "LCDMessageFreeOpt.hpp"
#include <stdlib.h>
#define CHECK_BIT(var, pos) (var & (1<<pos))

class LCDMessage{
	public:
		char* str;
		int str_len;
		int at_row;
		int idx = 0;
		bool played = false;
		int play_for_x_ticks = PLAY_FOR_TICKS_DISABLED;
		int user_flags = 0;
		bool enabled = true;
		bool is_static = false;
		LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR;
		
		~LCDMessage();
};
