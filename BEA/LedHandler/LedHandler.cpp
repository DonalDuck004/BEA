#include "LedHandler.hpp"

LedHandler::LedHandler(int pin, int update_delay) : BEAHandler(update_delay) {
    pinMode(pin, OUTPUT);
    this->pin = pin;
}

void LedHandler::SetOn() {
    this->UnsetDelay();
    digitalWrite(this->pin, HIGH);
}

void LedHandler::SetOff() {
    this->UnsetDelay();
    digitalWrite(this->pin, LOW);
}

void LedHandler::SetDelay(int delay) {
    this->update_delay = delay;
}

void LedHandler::UnsetDelay() {
    this->SetDelay(0);
}

void LedHandler::EnableBlink() {
    this->enabled = true;
}

void LedHandler::DisableBlink() {
    this->enabled = false;
}

bool LedHandler::ShouldTick() {
    return this->enabled && this->update_delay != 0 && BEAHandler::ShouldTick();
}

void LedHandler::DoUpdate() {
    BEAHandler::DoUpdate();
    digitalWrite(this->pin, this->powered);
    this->powered = !this->powered;
}

