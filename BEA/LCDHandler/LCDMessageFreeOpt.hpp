#pragma once

enum LCDMessageFreeOpt {
	NO_CLEAN			  = 0b0001,
	FREE_STR			  = 0b0010,
	PERSISTENT			  = 0b0100,
	REMOVE_ALL_PERSISTENT = 0b1000
};
