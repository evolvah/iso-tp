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

    // Set up the frame forwarder to send frames back to the buffer
    // object, using its RX functionality
    IsoTpBuffer                 isoTpBuffer;
    CanBusSenderFrameForwarder  frameForwarder(isoTpBuffer);

    // Loopback transmission
    isoTpBuffer.TransmitMessage(data, testByteCount, frameForwarder);

    // Let's see what we received
    printf("Dumping the rxBuffer...\n");
    isoTpBuffer.ShowRxBuffer();

    // Verify what we actually received
    byte    rxData[1000];
    uint rxByteCount = isoTpBuffer.FetchMessage(rxData, 1000);
    printf("Received %u bytes, verifying against the transmitted data\n", rxByteCount);
    for(uint i = 0; i < rxByteCount; ++i)
        assert(data[i] == rxData[i]);
    printf("All good...\n");

    return 0;
}