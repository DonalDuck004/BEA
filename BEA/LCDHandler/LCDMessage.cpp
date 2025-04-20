#include "LCDMessage.hpp"

#pragma region BaseLCDMessage
int BaseLCDMessage::GetAtRow() {
    return this->at_row;
}

byte BaseLCDMessage::GetFlags() {
    return this->user_flags;
}

LCDMessageFreeOpt BaseLCDMessage::GetFreeFlags() {
    return this->free_op;
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

#pragma endregion

#pragma region BaseLCDMessageText
BaseLCDMessageText::BaseLCDMessageText(int at_row, char* str, LCDMessageFreeOpt free_op, int priority) {
	this->str = str;
    if (str == nullptr)
        this->str_len = 0;
    else
        this->str_len = strlen(str);

	this->free_op = free_op;
    this->at_row = at_row;
    this->priority = priority;

    this->list_for_silent = false;
}

void BaseLCDMessageText::SetStr(char* str, bool update_len){
    this->str = str;
    if (update_len)
        SetStrLen(strlen(str));
}

void BaseLCDMessageText::SetStrLen(int len) {
    if (len < 0)
        return;

    this->str_len = len;
}

BaseLCDMessageText::~BaseLCDMessageText() {
	if (this->free_op == LCDMessageFreeOpt::FREE_STR)
		free(this->str);
}

void BaseLCDMessageText::Reset(bool recalculate_len) {
    if (recalculate_len)
        this->SetStrLen(strlen(this->str));

    this->enabled = true;
}

#pragma endregion

#pragma region LCDMessageText
LCDMessageText::LCDMessageText(int at_row, char* str, bool play_once, LCDMessageFreeOpt free_op, int priority) :
    BaseLCDMessageText(at_row, str, free_op, priority) {
    this->play_once = play_once;
}

void LCDMessageText::Reset(bool recalculate_len) {
    BaseLCDMessageText::Reset(recalculate_len);

    this->idx = 0;
}

bool LCDMessageText::GetPlayOnce() {
    return this->play_once;
}

void LCDMessageText::UpdateShort(int cols, LiquidCrystal* raw, int group_len) {
    int tmp = this->idx;
    int w = 0;

    while (true) {
        if (w + group_len > cols) {
            w += raw->write(this->str, cols - w);
            if (w < cols)
                raw->write(LCDMessageText::sep, cols - w);
            break;
        }

        if (tmp > this->str_len)
            w += raw->write(LCDMessageText::sep + (tmp - this->str_len));
        else {
            w += raw->write(this->str + tmp);
            w += raw->write(LCDMessageText::sep);
        }

        tmp = 0;
    }
}

void LCDMessageText::UpdateLong(int cols, LiquidCrystal* raw, int group_len) {
    int w = 0;

    if (this->idx < this->str_len)
        w = raw->write(this->str + this->idx, min(cols, this->str_len - idx));

    if (w < cols) {
        w += raw->write(LCDMessageText::sep, this->idx - this->str_len >= 0 ? -idx + group_len : LCDMessageText::sep_len);
        if (w < cols)
            raw->write(this->str, cols - w);
    }
}

void LCDMessageText::UpdateStatic(int cols, LiquidCrystal* raw) {
    raw->write(this->str, this->str_len);
    for (int i = this->str_len; i < cols; i++)
        raw->write(' ');
}

void LCDMessageText::DispatchUpdate(int cols, LiquidCrystal* raw, int group_len) {
    if (cols / this->str_len == 1)
        this->UpdateStatic(cols, raw);
    else if (group_len > cols)
        this->UpdateLong(cols, raw, group_len);
    else
        this->UpdateShort(cols, raw, group_len);
}

bool LCDMessageText::DoUpdate(LCDHandler* lcd) {
    if (this->str_len == 0) {
        lcd->ClearRow(this->at_row);
        return false;
    }

    int group_len = this->sep_len + this->str_len;
    LiquidCrystal* raw = lcd->GetRaw();

    raw->setCursor(0, this->at_row);

    this->DispatchUpdate(lcd->GetCols(), raw, group_len);

    int tmp = this->idx++;
    this->idx %= group_len;

    return tmp > this->idx && this->play_once;
}

#pragma endregion

#pragma region LCDMessageStaticText
LCDMessageStaticText::LCDMessageStaticText(int at_row, char* str, int play_for_x_ticks, LCDMessageFreeOpt free_op, int priority) :
    BaseLCDMessageText(at_row, str, free_op, priority) {
    this->play_for_x_ticks = play_for_x_ticks;
    this->src_play_for_x_ticks = play_for_x_ticks;
}

void LCDMessageStaticText::DoSilentUpdate(LCDHandler* lcd) {
    if (this->play_for_x_ticks == PLAY_FOR_TICKS_DISABLED)
        this->play_for_x_ticks--;
}

void LCDMessageStaticText::Reset(bool recalculate_len) {
    BaseLCDMessageText::Reset(recalculate_len);
    this->play_for_x_ticks = this->src_play_for_x_ticks;
}

bool LCDMessageStaticText::DoUpdate(LCDHandler* lcd) {
    int cols = lcd->GetCols();
    LiquidCrystal* raw = lcd->GetRaw();

    if (this->str_len < cols)
        lcd->ClearRow(this->at_row);

    raw->setCursor(0, this->at_row);
    raw->write(this->str, this->str_len);
    
    if (this->play_for_x_ticks == PLAY_FOR_TICKS_DISABLED)
        return false;

    return --this->play_for_x_ticks == 0;
}

#pragma endregion

#pragma region LCDMessageStaticLikeText
void LCDMessageStaticLikeText::DispatchUpdate(int cols, LiquidCrystal* raw, int group_len) {
    if (this->str_len <= cols)
        this->UpdateStatic(cols, raw);
    else if (group_len > cols)
        this->UpdateLong(cols, raw, group_len);
}

#pragma endregion
