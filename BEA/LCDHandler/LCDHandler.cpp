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
    
int LCDHandler::GetCols(){
    return this->cols;
}
    
int LCDHandler::GetRows(){
    return this->rows;
}
    
    
void LCDHandler::AddMessage(BaseLCDMessage* msg){
    this->messages_count++;
    if (this->messages == NULL){
        this->messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*));
    }else{
        this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);
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
            this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);
        else if (at == 0)
            this->messages = (BaseLCDMessage**)realloc(this->messages + sizeof(BaseLCDMessage*), sizeof(BaseLCDMessage*) * this->messages_count);
        else {
            for (int i = at; i < this->messages_count; i++)
                this->messages[i] = this->messages[i + 1];
        }
        this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);
    }
}

BaseLCDMessage* LCDHandler::GetMessageAt(int at) {
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

            if (this->messages[i]->DoUpdate(this)){
                this->ClearRow(this->messages[i]->GetAtRow());
                this->RemoveMessageAt(i);
                i--;
            }
        }
    }
}

LiquidCrystal* LCDHandler::GetRaw() {
    return this->lcd;
}
