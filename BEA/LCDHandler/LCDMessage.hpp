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
		bool enabled = true;

		virtual bool DoUpdate(LCDHandler* lcd) = 0;
		virtual void DoSilentUpdate(LCDHandler* lcd) = 0;

		int GetPriority();
		int GetAtRow();
		byte GetFlags();
		LCDMessageFreeOpt GetFreeFlags();
		bool GetListForSilent();

		BaseLCDMessage* SetFlags(byte flags);
		BaseLCDMessage* SetListForSilent(bool list_for_silent);

	protected:
		int priority = 0;
		byte user_flags = 0;
		int at_row;
		bool list_for_silent;
		LCDMessageFreeOpt free_op;
};


class BaseLCDMessageText : public BaseLCDMessage {
	public:
		BaseLCDMessageText(int at_row, char* str, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR, int priority = 0);

		~BaseLCDMessageText();
		virtual bool DoUpdate(LCDHandler* lcd) = 0;
		virtual void DoSilentUpdate(LCDHandler* lcd) {};
		virtual void Reset() {};
		void SetStrLen(int len);
		char* TMPGetText() { return this->str; }

	protected:
		char* str;
		int str_len;
};

class LCDMessageText : public BaseLCDMessageText {
	public:
		LCDMessageText(int at_row, char* str, bool play_once = false, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR, int priority = 0);

		bool GetPlayed();

		bool GetPlayOnce();

		bool DoUpdate(LCDHandler* lcd);
		
	protected:
		int idx = 0;
		bool played = false;
		bool play_once = false;
};

class LCDMessageStaticText : public BaseLCDMessageText {
	protected:
		int play_for_x_ticks;
		int src_play_for_x_ticks;

	public:
		LCDMessageStaticText(int at_row, char* str, int play_for_x_ticks = PLAY_FOR_TICKS_DISABLED, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR, int priority = 0);

		bool DoUpdate(LCDHandler* lcd);
		void DoSilentUpdate(LCDHandler* lcd);
		void Reset();
};

