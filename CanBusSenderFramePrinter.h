#ifndef CanBusTxFramePrinter_h_
#define CanBusTxFramePrinter_h_

#include "Utils.h"
#include "CanBusSender.h"

class CanBusSenderFramePrinter : public CanBusSender {
public:
	virtual void SendFrame(byte* src, uint len);
};

#endif
