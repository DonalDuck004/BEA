#include "LCDMessage.hpp"


bool LCDMessageText::GetPlayed() {
	return this->played;
}

bool LCDMessageText::GetPlayOnce() {
    return this->play_once;
}

int BaseLCDMessage::GetAtRow() {
    return this->at_row;
}

byte BaseLCDMessage::GetFlags() {
    return this->user_flags;
}

bool BaseLCDMessage::GetListForSilent() {
    return this->list_for_silent;
}

BaseLCDMessage* BaseLCDMessage::SetListForSilent(bool list_for_silent) {
    this->list_for_silent = list_for_silent;
    return this;
}

BaseLCDMessage* BaseLCDMessage::SetFlags(byte user_flags) {
    this->user_flags = user_flags;
    return this;
}

BaseLCDMessageText::BaseLCDMessageText(int at_row, char* str, LCDMessageFreeOpt free_op) {
	this->str = str;
	this->str_len = strlen(str);
	this->free_op = free_op;
    this->at_row = at_row;

    this->list_for_silent = false;
}

void BaseLCDMessageText::SetStrLen(int len) {
    if (len < 0)
        return;

    this->str_len = len;
}

BaseLCDMessageText::~BaseLCDMessageText() {
	if (CHECK_BIT(this->free_op, LCDMessageFreeOpt::FREE_STR))
		free(this->str);
}


LCDMessageText::LCDMessageText(int at_row, char* str, bool play_once, LCDMessageFreeOpt free_op) : BaseLCDMessageText(at_row, str, free_op) {
    this->play_once = play_once;
}

bool LCDMessageText::DoUpdate(LCDHandler* lcd) {
    int cols = lcd->GetCols();
    LiquidCrystal* raw = lcd->GetRaw();
    
    if (this->str_len < cols)
        lcd->ClearRow(this->at_row);

    raw->setCursor(0, this->at_row);

    if (this->idx == -1)
        raw->print(" ");

    if (this->idx < this->str_len)
        raw->print(str_span(this->str, this->idx < 0 ? 0 : this->idx));

    if (this->idx == 1)
        this->played = true;

    if (this->played && this->play_once)
        return true;

    int diff = this->str_len - this->idx;
    if (-3 < diff && diff < cols) {
        int to_fill = cols - diff;
        int x;

        if (to_fill == 1)
            x = 1;
        else if (to_fill == cols || to_fill == 2)
            x = 2;
        else
            x = 3;

        raw->print(str_repeat(" ", x));

        raw->print(str_span(this->str, 0, to_fill - x));
    }

    if (diff == 0)
        this->idx = -1;
    else
        this->idx++;

    return false;
}


LCDMessageStaticText::LCDMessageStaticText(int at_row, char* str, int play_for_x_ticks, LCDMessageFreeOpt free_op) :
    LCDMessageText(at_row, str, free_op) {
    this->play_for_x_ticks = play_for_x_ticks;
}

void LCDMessageStaticText::DoSilentUpdate(LCDHandler* lcd) {
    if (this->play_for_x_ticks == PLAY_FOR_TICKS_DISABLED)
        this->play_for_x_ticks--;
}

bool LCDMessageStaticText::DoUpdate(LCDHandler* lcd) {
    int cols = lcd->GetCols();
    LiquidCrystal* raw = lcd->GetRaw();

    if (this->str_len < cols)
        lcd->ClearRow(this->at_row);


    raw->setCursor(0, this->at_row);
    raw->print(str_span(this->str, 0, this->str_len - 1));
    
    if (this->play_for_x_ticks == PLAY_FOR_TICKS_DISABLED)
        return false;
    else
        return --this->play_for_x_ticks == 0;
}


LCDMessageStaticCustomChar::LCDMessageStaticCustomChar(int at_row, byte custom_char) {
    this->at_row = at_row;
    this->custom_char = custom_char;
}
