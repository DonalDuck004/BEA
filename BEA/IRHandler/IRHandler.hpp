#pragma once
#include "../BEAHandler/BEAHandler.hpp"
#include <LiquidCrystal.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

#ifndef IR_PIN
#   define IR_PIN 34
#endif
#ifndef IR_TICK_DELAY
#   define IR_TICK_DELAY 250
#endif
class IRHandler;

typedef void IRHandlerCBK(IRHandler*, uint16_t);

class IRHandler : public BEAHandler{
    private:
        IRrecv* ir;
        IRHandlerCBK* cbk;

    protected:
        virtual void DoUpdate() override;

    public:
        bool playing = false;

        IRHandler(IRHandlerCBK cbk, int pin = IR_PIN, int update_delay = IR_TICK_DELAY);
        IRHandler(int pin = IR_PIN, int update_delay = IR_TICK_DELAY);

        ~IRHandler();

        void DeviceConnected();
       
        void SetCallback(IRHandlerCBK cbk);

        bool Tick() override;
};

