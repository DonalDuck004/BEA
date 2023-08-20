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
#ifndef IR_DISCONNECT
#   define IR_DISCONNECT 69
#endif
#ifndef IR_VOL_PLUS
#   define IR_VOL_PLUS 70
#endif
#ifndef IR_VOL_MINUS
#   define IR_VOL_MINUS 21
#endif
#ifndef IR_PAUSE
#   define IR_PAUSE 64
#endif
#ifndef IR_NEXT
#   define IR_NEXT 67
#endif
#ifndef IR_BACK
#   define IR_BACK 68
#endif
#ifndef IR_SPARA_STRONZATA
#   define IR_SPARA_STRONZATA 22
#endif
#ifndef IR_HEAP_DEBUG
#   define IR_HEAP_DEBUG 25
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

