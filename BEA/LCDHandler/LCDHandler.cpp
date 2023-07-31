#include "LCDHandler.hpp"

LCDHandler::LCDHandler(LiquidCrystal* lcd, int cols, int rows, int update_delay) : BEAHandler(update_delay) {
    this->lcd = lcd;
    this->cols = cols;
    this->rows = rows;
    this->messages = NULL;
    this->messages_count = 0;

    lcd->begin(cols, rows);
    lcd->clear();
}

LCDHandler::~LCDHandler(){
  this->RemoveMessages();
}

bool LCDHandler::UpdateMessage(LCDMessage* msg) {
    if (msg->str_len < this->cols)
        this->ClearRow(msg->at_row);

    this->lcd->setCursor(0, msg->at_row);

    if (!msg->is_static && msg->idx == -1)
        this->lcd->print(" ");

    if (!msg->is_static && msg->idx < msg->str_len)
        this->lcd->print(str_span(msg->str, msg->idx < 0 ? 0 : msg->idx));

    if (msg->play_for_x_ticks == 0)
        return true;
    else if (msg->idx == 1)
        msg->played = true;

    if (msg->play_for_x_ticks != PLAY_FOR_TICKS_DISABLED)
        msg->play_for_x_ticks--;

    int diff = msg->str_len - msg->idx;
    if (!msg->is_static && -3 < diff && diff < this->cols) {
        int to_fill = this->cols - diff;
        int x;

        if (to_fill == 1)
            x = 1;
        else if (to_fill == this->cols || to_fill == 2)
            x = 2;
        else
            x = 3;

        this->lcd->print(str_repeat(" ", x));

        this->lcd->print(str_span(msg->str, 0, to_fill - x));
    }

    if (diff == 0)
        msg->idx = -1;
    else
        msg->idx++;

    if (msg->is_static)
        this->lcd->print(msg->str);

    return false;
}
    
int LCDHandler::GetCols(){
    return this->cols;
}
    
int LCDHandler::GetRows(){
    return this->rows;
}
    
void LCDHandler::AddMessage(char* text, int at, int play_for_x_ticks, bool is_static, int user_flags, LCDMessageFreeOpt free_opt){
    LCDMessage* msg = new LCDMessage();
    msg->str = text;
    msg->str_len = strlen(text);
    msg->at_row = at;
    msg->play_for_x_ticks = play_for_x_ticks;
    msg->user_flags = user_flags;
    msg->is_static = is_static;
    msg->free_op = free_opt;

    this->AddMessage(msg);
}
    
void LCDHandler::AddMessage(LCDMessage* msg){
    this->messages_count++;
    if (this->messages == NULL){
        this->messages = (LCDMessage**)malloc(sizeof(LCDMessage*));
    }else{
        this->messages = (LCDMessage**)realloc(this->messages, sizeof(LCDMessage*) * this->messages_count);
    }

    this->messages[this->messages_count - 1] = msg;
}
    
void LCDHandler::RemoveMessageAt(int at){
    if(this->messages_count <= at || at < 0)
        return;

    this->messages_count--;
    if (this->messages_count == 0){
        delete this->messages[0];
        free(this->messages);
        this->messages = NULL;
    }else{
        delete this->messages[at];
        // TODO CHECK
        if (at == this->messages_count)
            this->messages = (LCDMessage**)realloc(this->messages, sizeof(LCDMessage*) * this->messages_count);
        else if (at == 0)
            this->messages = (LCDMessage**)realloc(this->messages + sizeof(LCDMessage*), sizeof(LCDMessage*) * this->messages_count);
        else {
            for (int i = at; i < this->messages_count; i++)
                this->messages[i] = this->messages[i + 1];
        }
        this->messages = (LCDMessage**)realloc(this->messages, sizeof(LCDMessage*) * this->messages_count);
    }
}

LCDMessage* LCDHandler::GetMessageAt(int at) {
    if (this->messages_count <= at || at < 0)
        return NULL;

    return this->messages[at];
}
    
void LCDHandler::RemoveMessages(){
    if (this->messages == NULL)
        return;

    this->messages_count = 0;
    delete[] this->messages;
    this->messages = NULL;
}
    
void LCDHandler::ClearAll(){
    this->lcd->clear();
}
    
void LCDHandler::ForceWrite(char* text, int at, int col){
    this->ClearRow(at);
    this->lcd->setCursor(col, at);
    this->lcd->print(text);    
}
    
void LCDHandler::ClearRow(int at){
    this->lcd->setCursor(0, at);
    this->lcd->print(str_repeat(" ", this->cols));
}
    
void LCDHandler::DoUpdate(){
    if (this->messages_count != 0){
        BEAHandler::DoUpdate();

        for(int i = 0; i < this->messages_count; i++){
            if (!this->messages[i]->enabled)
                continue;

            if (this->UpdateMessage(this->messages[i])){
                this->ClearRow(this->messages[i]->at_row);
                this->RemoveMessageAt(i);
                i--;
            }
        }
    }
}
