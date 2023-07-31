#include "IRHandler.hpp"

IRHandler::IRHandler(int pin, IRHandlerCBK* cbk, int update_delay) : BEAHandler(update_delay){
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

void IRHandler::Tick() {
    if (!this->ShouldTick())
        return;

    if (this->ir->decode()) {
        BEAHandler::DoUpdate();
        this->ir->resume();
        this->DoUpdate();
    }
}
