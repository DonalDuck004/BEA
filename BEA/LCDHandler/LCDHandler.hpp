#pragma once

#include <LiquidCrystal.h>
#include "LCDMessage.hpp"
#include "LCDMessageFreeOpt.hpp"
#include "../string_utilies/string_utilies.hpp"
#include "../BEAHandler/BEAHandler.hpp"
class BaseLCDMessage;

class LCDHandler : public BEAHandler{
	private:
		LiquidCrystal* lcd;
		int cols;
		int rows;
		int repeat_after;
		int messages_count;
		BaseLCDMessage **messages;

	protected:
		void DoUpdate() override;

	public:
		LCDHandler(LiquidCrystal* lcd, int cols = 16, int rows = 2, int update_delay = 1000);

		~LCDHandler();

		int GetCols();

		int GetRows();

		void AddMessage(BaseLCDMessage* msg);

		void RemoveMessageAt(int at);

		BaseLCDMessage* GetMessageAt(int at);

		void RemoveMessages();

		void ClearAll();

		void ForceWrite(char* text, int at = 0, int col = 0);

		void ClearRow(int at = 0);

		LiquidCrystal* GetRaw();
};
