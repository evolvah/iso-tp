#ifndef CanBusSenderFrameForwarder_h_
#define CanBusSenderFrameForwarder_h_

#include "Utils.h"
#include "CanBusSender.h"
#include "IsoTpBuffer.h"

class CanBusSenderFrameForwarder : public CanBusSender {
    IsoTpBuffer&    myRxBuff;
public:
    CanBusSenderFrameForwarder(IsoTpBuffer& rxBuff);
    virtual void SendFrame(byte* src, uint len);
};

#endif
