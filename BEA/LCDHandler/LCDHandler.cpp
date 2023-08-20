#include "LCDHandler.hpp"

LCDHandler::LCDHandler(LiquidCrystal* lcd, int cols, int rows, int update_delay) : BEAHandler(update_delay) {
    this->lcd = lcd;
    this->cols = cols;
    this->rows = rows;
    this->messages = nullptr;
    this->messages_count = 0;
    this->space_for_messages = 0;

    lcd->begin(cols, rows);
    lcd->clear();
}

LCDHandler::LCDHandler(int cols, int rows, int update_delay) : BEAHandler(update_delay) {
    this->lcd = new LiquidCrystal(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN);
    this->cols = cols;
    this->rows = rows;
    this->messages = nullptr;
    this->messages_count = 0;
    this->space_for_messages = 0;

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
    if (this->messages == nullptr) {
        this->messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*) * LCDHANDLER_INTERNAL_MESSAGES_BUFF);
        this->space_for_messages = LCDHANDLER_INTERNAL_MESSAGES_BUFF;
    }else if (this->space_for_messages == this->messages_count - 1) {
        this->space_for_messages += LCDHANDLER_INTERNAL_EXTRA_MESSAGES_BUFF;
        this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->space_for_messages);
    }
    
    this->messages[this->messages_count - 1] = msg;

    if (this->messages_count != 1)
        this->SortMessages();
}
    
void LCDHandler::RemoveMessageAt(int at){
    if(this->messages_count <= at || at < 0)
        return;

    this->messages_count--;
    /*ClearRow(this->messages[at]->GetAtRow());
    lcd->setCursor(0, this->messages[at]->GetAtRow());
    lcd->print(this->messages[at]->GetFreeFlags());
    lcd->print(((BaseLCDMessageText*)this->messages[at])->TMPGetText());*/
    delete this->messages[at];
    this->messages[at] = nullptr;
    if (this->messages_count == 0)
        return;

    for (int i = at; i < this->messages_count; i++)
        this->messages[i] = this->messages[i + 1];
}

void LCDHandler::ReallocBuff() {
    if (this->messages_count == this->space_for_messages)
        return;

    this->space_for_messages = this->messages_count;
    this->messages = (BaseLCDMessage**)realloc(this->messages, sizeof(BaseLCDMessage*) * this->messages_count);
}

BaseLCDMessage* LCDHandler::GetMessageAt(int at) {
    if (this->messages_count <= at || at < 0)
        return nullptr;

    return this->messages[at];
}
    
void LCDHandler::RemoveMessages(){
    if (this->messages == nullptr)
        return;

    int original_count = this->messages_count;
    for (int i = this->messages_count - 1; i >= 0; i--)
    {
        if (this->messages[i]->GetFreeFlags() != LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT)
        {
            delete this->messages[i];
            this->messages[i] = nullptr;
            this->messages_count--;
        }
    }

    this->SortMessages(original_count);
}

void LCDHandler::SortMessages(int messages_count) {
    if (messages_count == -1)
        messages_count = this->messages_count;
    else if (messages_count == -2)
        messages_count = this->space_for_messages;

    if (messages_count == 0)
        return;
    if (messages_count == 1) {
        for (int i = 0; i < messages_count; i++) {
            if (this->messages[i] != nullptr) {
                this->messages[0] = this->messages[i];
                this->messages[i] = nullptr;
                break;
            }
        }

        return;
    }

    BaseLCDMessage* tmp;
    for (int i = 0; i < messages_count - 1; i++) {
        for (int j = 0; j < messages_count - i - 1; j++) {
            if (this->messages[j + 1] == nullptr)
                continue;

            if (this->messages[j] == nullptr || this->messages[j]->GetPriority() > this->messages[j + 1]->GetPriority())
                std::swap(this->messages[j], this->messages[j + 1]);
        }
    }
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
    static char* tmp = str_repeat(" ", this->cols);
    this->lcd->print(tmp);
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

void LCDHandler::GetLastMessagesForRows(LCDMessageGroup& output) {
    int rows = this->GetRows();
    output.count = 0;

    if (output.messages == nullptr) {
        output.messages = (BaseLCDMessage**)malloc(sizeof(BaseLCDMessage*) * rows);
        output.buff_size = rows;
    } else if (output.buff_size < rows) {
        output.messages = (BaseLCDMessage**)realloc(output.messages, sizeof(BaseLCDMessage*) * rows);
        output.buff_size = rows;
    }

    memset(output.messages, NULL, sizeof(BaseLCDMessage*) * output.buff_size);

    for (int i = this->messages_count - 1; i >= 0; i--) {
        if (this->messages[i]->enabled && output.messages[this->messages[i]->GetAtRow()] == NULL) {
            output.messages[this->messages[i]->GetAtRow()] = this->messages[i];
            output.count++;

            if (output.count == rows)
                break;
        }
    }
}

LCDMessageGroup LCDHandler::GetLastMessagesForRows() {
    LCDMessageGroup out;
    this->GetLastMessagesForRows(out);

    return out;
}

void LCDHandler::DoUpdate() {
    if (this->messages_count != 0) {
        BEAHandler::DoUpdate();

#ifdef LCD_LEGACY_UPDATER
        for (int i = 0; i < this->messages_count; i++) {
            if (!this->messages[i]->enabled)
                continue;

            if (this->messages[i]->DoUpdate(this)) {
                this->ClearRow(this->messages[i]->GetAtRow());
                this->RemoveMessageAt(i);
                i--;
            }
        }
#else
        static LCDMessageGroup msgs;
        this->GetLastMessagesForRows(msgs);

        bool tmp;

        for (int i = 0; i < this->messages_count; i++) {
            continue;
            if (!(this->messages[i]->enabled && this->messages[i]->GetListForSilent()))
                continue;
            tmp = false;
            // TODO Make GetListForSilent returns an enum values. DO_SILENT and DO_SILENT_ONLY_WHEN_NOT_BEING_UPDATED(maybe the name must be more short lol)
            // If DO_SILENT_ONLY_WHEN_NOT_BEING_UPDATED do the bottom cycle
            for (int j = 0; j < msgs.count; j++) {
                if (msgs.messages[j] == this->messages[i]) {
                    tmp = true;
                    break;
                }
            }

            if (!tmp)
                this->messages[i]->DoSilentUpdate(this);
        }


        for (int i = 0; i < msgs.count; i++) {
            if (msgs.messages[i]->DoUpdate(this)) {
                switch (msgs.messages[i]->GetFreeFlags())
                {
                    case LCDMessageFreeOpt::PERSISTENT:
                    case LCDMessageFreeOpt::REMOVE_ALL_PERSISTENT:
                        msgs.messages[i]->enabled = false;
                        break;
                    default:
                        lcd->clear();
                        lcd->print("Removed :D");
                        delay(10000);
                        lcd->print(this->messages[i]->GetFreeFlags());
                        delay(10000);
                        lcd->print(this->messages[i]->GetFlags());
                        delay(10000);
                        this->RemoveMessageByRef(msgs.messages[i]);
                        break;
                }
            }
        }
#endif
    }
}

