#pragma once

#define PLAY_FOR_TICKS_DISABLED -1
#include "LCDMessageFreeOpt.hpp"
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <stdlib.h>
#include "LCDHandler.hpp"

class LCDHandler;
#define CHECK_BIT(var, pos) (var & (1<<pos))

class BaseLCDMessage {
	public:
		byte user_flags = 0;
		bool enabled = true;

		virtual bool DoUpdate(LCDHandler* lcd) = 0;
		BaseLCDMessage* SetFlags(byte flags);

		int GetAtRow();

	protected:
		int at_row;
};


class BaseLCDMessageText : public BaseLCDMessage {
	public:
		BaseLCDMessageText(int at_row, char* str, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR);

		~BaseLCDMessageText();
		virtual bool DoUpdate(LCDHandler* lcd) = 0;
		void SetStrLen(int len);

	protected:
		char* str;
		int str_len;
		LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR;
};

class LCDMessageText : public BaseLCDMessageText {
	public:
		LCDMessageText(int at_row, char* str, bool play_once = false, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR);

		bool GetPlayed();

		bool GetPlayOnce();

		bool DoUpdate(LCDHandler* lcd);

	protected:
		int idx = 0;
		bool played = false;
		bool play_once = false;
};

class LCDMessageStaticText : public LCDMessageText {
	protected:
		int play_for_x_ticks;

	public:
		LCDMessageStaticText(int at_row, char* str, int play_for_x_ticks = PLAY_FOR_TICKS_DISABLED, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR);

		bool DoUpdate(LCDHandler* lcd);
};

class LCDMessageStaticCustomChar : public BaseLCDMessage {
	protected:
		byte custom_char;

	public:
		LCDMessageStaticCustomChar(int at_row, byte custom_char);
};
