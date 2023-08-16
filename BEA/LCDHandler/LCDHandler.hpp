#pragma once

#include <LiquidCrystal.h>
#include "LCDMessage.hpp"
#include "LCDMessageFreeOpt.hpp"
#include "../string_utilies/string_utilies.hpp"
#include "../BEAHandler/BEAHandler.hpp"

#ifndef LCD_RS_PIN
#	define LCD_RS_PIN 19
#endif
#ifndef LCD_ENABLE_PIN
#	define LCD_ENABLE_PIN 23
#endif
#ifndef LCD_D0_PIN
#	define LCD_D0_PIN 18
#endif
#ifndef LCD_D1_PIN
#	define LCD_D1_PIN 17
#endif
#ifndef LCD_D2_PIN
#	define LCD_D2_PIN 16
#endif
#ifndef LCD_D3_PIN
#	define LCD_D3_PIN 15
#endif
#ifndef LCD_ROWS
#	define LCD_ROWS 2
#endif
#ifndef LCD_COLS
#	define LCD_COLS 16
#endif
#ifndef LCD_TICK_DELAY
#	define LCD_TICK_DELAY 750
#endif
#ifndef LCDHANDLER_GROUP_BASE_BUFF
#	define LCDHANDLER_GROUP_BASE_BUFF 2
#endif
#ifndef LCDHANDLER_INTERNAL_MESSAGES_BUFF
#	define LCDHANDLER_INTERNAL_MESSAGES_BUFF 4
#endif
#ifndef LCDHANDLER_INTERNAL_EXTRA_MESSAGES_BUFF
#	define LCDHANDLER_INTERNAL_EXTRA_MESSAGES_BUFF 2
#endif


class BaseLCDMessage;

typedef struct {
	BaseLCDMessage** messages = nullptr;
	int count = 0;
	int buff_size = 0;
} LCDMessageGroup;

class LCDHandler : public BEAHandler{
	private:
		LiquidCrystal* lcd;
		int cols;
		int rows;
		int repeat_after;
		int messages_count;
		int space_for_messages;
		BaseLCDMessage **messages;

		static inline bool CheckFlags(int mf, int f, bool s);

	protected:
		void DoUpdate() override;
		void SortMessages(int messages_count = -1);

	public:
		LCDHandler(LiquidCrystal* lcd, int cols = LCD_COLS, int rows = LCD_ROWS, int update_delay = LCD_TICK_DELAY);

		LCDHandler(int cols = LCD_COLS, int rows = LCD_ROWS, int update_delay = LCD_TICK_DELAY);

		~LCDHandler();

		int GetCols();

		int GetRows();

		void AddMessage(BaseLCDMessage* msg);

		void RemoveMessageAt(int at);

		BaseLCDMessage* GetMessageAt(int at);

		LCDMessageGroup GetMessagesWithFlags(byte flags, bool strict = false);
		
		LCDMessageGroup GetLastMessagesForRows(bool& any_silent);
		void GetLastMessagesForRows(bool& any_silent, LCDMessageGroup& output);

		LCDMessageGroup GetMessagesAtRow(int row);
		
		bool RemoveMessagesAtRow(int row);

		bool RemoveMessageByRef(BaseLCDMessage* msg);

		bool RemoveMessagesWithFlags(byte flags, bool strict = false, int limit = -1);

		void RemoveMessages();

		void ClearAll();

		void ForceWrite(char* text, int at = 0, int col = 0);

		void ClearRow(int at = 0);

		LiquidCrystal* GetRaw();

		void ReallocBuff();
};
