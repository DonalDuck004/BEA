#pragma once
#include "../BEAHandler/BEAHandler.hpp"
#include <LiquidCrystal.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

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

        IRHandler(int pin, IRHandlerCBK cbk, int update_delay = 750);
        IRHandler(int pin, int update_delay = 750);

        ~IRHandler();

        void DeviceConnected();
       
        void SetCallback(IRHandlerCBK cbk);

        void Tick() override;
};

