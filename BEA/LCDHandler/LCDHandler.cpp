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
    if (this->messages == NULL)
        this->messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*));
    else
        this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);

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

        if (at == this->messages_count)
            this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);
        else if (at == 0)
            this->messages = (BaseLCDMessage**)realloc(this->messages + sizeof(BaseLCDMessage*), sizeof(BaseLCDMessage*) * this->messages_count);
        else
            for (int i = at; i < this->messages_count; i++)
                this->messages[i] = this->messages[i + 1];

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

#ifdef LCD_LEGACY_UPDATER
        for(int i = 0; i < this->messages_count; i++){
            if (!this->messages[i]->enabled)
                continue;

            if (this->messages[i]->DoUpdate(this)){
                this->ClearRow(this->messages[i]->GetAtRow());
                this->RemoveMessageAt(i);
                i--;
            }
        }
#else
        bool any_silent;
        LCDMessageGroup msgs = this->GetLastMessagesForRows(any_silent);
        if (any_silent) {
            bool tmp;

            for (int i = 0; i < this->messages_count; i++) {
                if (!this->messages[i]->enabled || !this->messages[i]->GetListForSilent())
                    continue;
                tmp = false;

                for (int j = 0; j < msgs.count; j++) {
                    if (msgs.messages[j] == this->messages[i]) {
                        tmp = true;
                        break;
                    }
                }

                if (!tmp)
                    this->messages[i]->DoSilentUpdate(this);
            }
        }

        for (int i = 0; i < msgs.count; i++) {

            if (msgs.messages[i]->DoUpdate(this))
                this->RemoveMessageByRef(msgs.messages[i]); // TODO FIX CRASH 
        }
#endif
    }
}

LiquidCrystal* LCDHandler::GetRaw() {
    return this->lcd;
}

bool LCDHandler::CheckFlags(int mf, int f, bool s) {
    return (s && mf == f) || (!s && (mf & f) == f);
}

bool LCDHandler::RemoveMessagesAtRow(int row) {
    bool deleted = false;

    for (int i = 0; i < this->messages_count; i++) {
        if (this->messages[i]->GetAtRow() == row) {
            this->RemoveMessageAt(i);
            i--;
            deleted = true;
        }
    }

    return deleted;
}

bool LCDHandler::RemoveMessageByRef(BaseLCDMessage* msg) {
    for (int i = 0; i < this->messages_count; i++) {
        if (this->messages[i] == msg) {
            this->RemoveMessageAt(i);
            return true;
        }
    }

    return false;
}

bool LCDHandler::RemoveMessagesWithFlags(byte flags, bool strict, int limit) {
    bool deleted = false;

    for (int i = 0; i < this->messages_count; i++) {
        if (CheckFlags(this->messages[i]->GetFlags(), flags, strict)) {
            this->RemoveMessageAt(i);
            i--;
            deleted = true;

            if (--limit == 0)
                break;
        }
    }

    return deleted;
}

LCDMessageGroup LCDHandler::GetMessagesAtRow(int row) {
    int found = 0;
    LCDMessageGroup out;

    out.messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*) * out.count);

    for (int i = 0; i < this->messages_count; i++) {
        if (this->messages[i]->GetAtRow() == row) {
            if (++found > out.count)
                out.messages = (BaseLCDMessage**)realloc(out.messages, sizeof(BaseLCDMessage*) * ++out.count);

            out.messages[found - 1] = this->messages[i];
        }
    }

    if (out.count != found)
    {
        out.count = found;
        out.messages = (BaseLCDMessage**)realloc(out.messages, sizeof(BaseLCDMessage*) * found);
    }

    return out;
}

LCDMessageGroup LCDHandler::GetMessagesWithFlags(byte flags, bool strict) {
    int found = 0;
    LCDMessageGroup out;

    out.messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*) * out.count);

    for (int i = 0; i < this->messages_count; i++) {
        if (CheckFlags(this->messages[i]->GetFlags(), flags, strict)) {

            if (++found > out.count)
                out.messages = (BaseLCDMessage**)realloc(out.messages, sizeof(BaseLCDMessage*) * ++out.count);

            out.messages[found - 1] = this->messages[i];
        }
    }

    if (out.count != found)
    {
        out.count = found;
        out.messages = (BaseLCDMessage**)realloc(out.messages, sizeof(BaseLCDMessage*) * found);
    }

    return out;
}

LCDMessageGroup LCDHandler::GetLastMessagesForRows(bool &any_silent) {
    int found = 0;
    LCDMessageGroup out;
    out.count = this->GetRows();

    short* indexes = (short*)malloc(sizeof(short) * out.count);
    memset(indexes, -1, sizeof(short) * out.count);

    for (int i = 0; i < this->messages_count; i++) {
        if (this->messages[i]->enabled) {
            indexes[this->messages[i]->GetAtRow()] = i;
            any_silent = any_silent || this->messages[i]->GetListForSilent();
        }
    }


    out.messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*) * out.count);

    for (int i = 0; i < out.count; i++) {
        if (indexes[i] == -1)
            continue;

        out.messages[found++] = this->messages[indexes[i]];
    }

    free(indexes);
    if (out.count != found)
    {
        out.count = found;
        out.messages = (BaseLCDMessage**)realloc(out.messages, sizeof(BaseLCDMessage*) * found);
    }

    return out;
}
