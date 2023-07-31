#include "LCDMessage.hpp"


LCDMessage::~LCDMessage() {
	if (CHECK_BIT(this->free_op, LCDMessageFreeOpt::FREE_STR))
		free(this->str);
}
