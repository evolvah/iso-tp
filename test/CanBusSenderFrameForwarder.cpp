#include <stdio.h>

#include "CanBusSenderFrameForwarder.h"

CanBusSenderFrameForwarder::CanBusSenderFrameForwarder(IsoTpBuffer& rxBuff) :
 	myRxBuff(rxBuff) {
}

void CanBusSenderFrameForwarder::SendFrame(byte* src, uint len) {
	myRxBuff.ProcessRxFrame(src, 8);
}

