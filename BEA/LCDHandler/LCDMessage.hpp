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
		int priority = 0;

		virtual bool DoUpdate(LCDHandler* lcd) = 0;
		virtual void DoSilentUpdate(LCDHandler* lcd) = 0;

		int GetAtRow();
		byte GetFlags();
		LCDMessageFreeOpt GetFreeFlags();
		bool GetListForSilent();

		BaseLCDMessage* SetFlags(byte flags);
		BaseLCDMessage* SetListForSilent(bool list_for_silent);

	protected:
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
		virtual void Reset(bool recalculate_len = true);
		void SetStr(char* str, bool update_len = true);
		void SetStrLen(int len);

	protected:
		char* str;
		int str_len;
		
		const char* sep = "   ";
		const int sep_len = strlen(sep);
};

enum LCDMessageTextProcessOpt {
	PURE_SCROLL,
	STATIC_LIKE,
};

class LCDMessageText : public BaseLCDMessageText {
	public:
		LCDMessageText(int at_row, 
					   char* str,
					   bool play_once = false,
					   LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR,
					   int priority = 0
			);


		bool GetPlayOnce();
		bool DoUpdate(LCDHandler* lcd);
		void Reset(bool recalculate_len = true) override;
		
	protected:
		int idx = 0;
		bool play_once = false;

		void UpdateShort(int cols, LiquidCrystal* raw, int group_len);
		void UpdateLong(int cols, LiquidCrystal* raw, int group_len);
		void UpdateStatic(int cols, LiquidCrystal* raw);

		virtual void DispatchUpdate(int cols, LiquidCrystal* raw, int group_len);
};

class LCDMessageStaticLikeText : public LCDMessageText {
	public:
		LCDMessageStaticLikeText(int at_row,
			char* str,
			bool play_once = false,
			LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR,
			int priority = 0
		) : LCDMessageText(at_row, str, play_once, free_op, priority) {};
	protected:
		virtual void DispatchUpdate(int cols, LiquidCrystal* raw, int group_len) override;
};


class LCDMessageStaticText : public BaseLCDMessageText {
	protected:
		int play_for_x_ticks;

	public:
		int src_play_for_x_ticks;
		
		LCDMessageStaticText(int at_row, char* str, int play_for_x_ticks = PLAY_FOR_TICKS_DISABLED, LCDMessageFreeOpt free_op = LCDMessageFreeOpt::FREE_STR, int priority = 0);

		bool DoUpdate(LCDHandler* lcd);
		void DoSilentUpdate(LCDHandler* lcd);
		void Reset(bool recalculate_len = true) override;
};

