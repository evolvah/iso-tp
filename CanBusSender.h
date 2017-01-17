#ifndef CanBusSender_h_
#define CanBusSender_h_

#include "Utils.h"

class CanBusSender {
public:
	virtual void SendFrame(byte* src, uint len) = 0;
};

#endif
