#pragma once

#include <Arduino.h>
#include "../BEAHandler/BEAHandler.hpp"

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

