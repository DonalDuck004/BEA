#pragma once

#include <Arduino.h>

#ifndef BEAHANDLER_DEAFULT_PICK_DELAY
#	define BEAHANDLER_DEAFULT_UPDATE_DELAY 750
#endif

class BEAHandler {
	protected:
		int update_delay;
		unsigned long last_update;
		virtual void DoUpdate();
		virtual bool ShouldTick();

	public:
		virtual bool Tick();

		BEAHandler(int pick_delay = BEAHANDLER_DEAFULT_UPDATE_DELAY);
};

