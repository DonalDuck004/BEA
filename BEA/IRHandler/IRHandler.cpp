#include "IRHandler.hpp"

IRHandler::IRHandler(IRHandlerCBK* cbk, int pin, int update_delay) : BEAHandler(update_delay){
    this->ir = new IRrecv(pin);
    this->ir->enableIRIn();
    this->cbk = cbk;
}

IRHandler::IRHandler(int pin, int update_delay) : BEAHandler(update_delay) {
    this->ir = new IRrecv(pin);
    this->ir->enableIRIn();
}

IRHandler::~IRHandler() {
    delete this->ir;
}

void IRHandler::DeviceConnected() {
    this->playing = false;
}

void IRHandler::SetCallback(IRHandlerCBK* cbk) {
    this->cbk = cbk;
}

void IRHandler::DoUpdate() {
    if (this->cbk != NULL)
        this->cbk(this, this->ir->decodedIRData.command);
}

bool IRHandler::Tick() {
    if (!this->ShouldTick())
        return false;

    if (this->ir->decode()) {
        BEAHandler::DoUpdate();
        this->ir->resume();
        this->DoUpdate();
    }

    return true;
}
