#pragma once

#include <Arduino.h>
#include "../BEAHandler/BEAHandler.hpp"

#ifndef BL_CONNECTING_BLINK_DELAY
#	define BL_CONNECTING_BLINK_DELAY 250
#endif

#ifndef BL_DISCONNECTED_BLINK_DELAY
#	define BL_DISCONNECTED_BLINK_DELAY 500
#endif

#ifndef BL_DISCONNECTING_BLINK_DELAY
#	define BL_DISCONNECTING_BLINK_DELAY 750
#endif

class LedHandler : public BEAHandler{
	private:
		int pin;
		bool powered = false;
		bool enabled = true;

	protected:
		void DoUpdate() override;
		bool ShouldTick() override;

	public:
		LedHandler(int pin, int update_delay = BEAHANDLER_DEAFULT_UPDATE_DELAY);

		void SetOn();

		void SetOff();

		void SetDelay(int delay);

		void UnsetDelay();

		void EnableBlink();

		void DisableBlink();
};

