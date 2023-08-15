#pragma once
#include "BEAHandler.hpp"

BEAHandler::BEAHandler(int update_delay) {
	this->update_delay = update_delay;
}

void BEAHandler::DoUpdate() {
	this->last_update = millis();
}

bool BEAHandler::ShouldTick() {
	return millis() - this->last_update >= this->update_delay;
}

bool BEAHandler::Tick() {
	if (this->ShouldTick()) {
		this->DoUpdate();
		return true;
	}

	return false;
}

