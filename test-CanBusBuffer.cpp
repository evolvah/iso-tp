#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "IsoTpBuffer.h"
#include "CanBusSenderFramePrinter.h"
#include "CanBusSenderFrameForwarder.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Bad argument!\n");
        return 1;
    }

    uint testByteCount = atoi(argv[1]);
    byte* data = new byte [testByteCount];
    for (uint i = 0; i < testByteCount; ++i)
        data[i] = i;

    IsoTpBuffer                 txBuffer;
    IsoTpBuffer                 rxBuffer;
    CanBusSenderFrameForwarder  frameForwarder(rxBuffer);

    txBuffer.TransmitMessage(data, testByteCount, frameForwarder);

    // Let's see what we received
    printf("Dumping the rxBuffer...\n");
    rxBuffer.ShowRxBuffer();

    byte    rxData[1000];
    uint rxByteCount = rxBuffer.FetchMessage(rxData, 1000);
    printf("Received %u bytes, verifying against the transmitted data\n", rxByteCount);
    for(uint i = 0; i < rxByteCount; ++i)
        assert(data[i] == rxData[i]);
    printf("All good...\n");

    return 0;
}