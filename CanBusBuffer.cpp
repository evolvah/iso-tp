#include <string.h>
#include <stdio.h>

#include "CanBusBuffer.h"

CanBusBuffer::CanBusBuffer() {
    Clear();
}

CanBusBuffer::~CanBusBuffer() {
}

void CanBusBuffer::Clear() {
    memset(buff, 0, MAX_ISOTP_BUFFER_LENGTH);
    bufferOverrun = false;
    bytesUsed = 0;
    framesStored = 0;
}

uint CanBusBuffer::EncodeMessage(byte* src, uint len) {
    // Let's do a sanity check first
    if (len == 0 || len > MAX_APPLICATION_MESSAGE_LENGTH)
        return 0;

    Clear();
    if (len > MAX_ISOTP_SINGLE_FRAME_PAYLOAD_LENGTH)
        return EncodeMultiFrameMessage(src, len);

    // Encode as a Single Frame
    buff[0] = len;
    memcpy(buff+1, src, len);
    framesStored = 1;
    bytesUsed = framesStored * CANBUS_FRAME_SIZE;
    return bytesUsed;
}

/* private */
uint CanBusBuffer::EncodeMultiFrameMessage(byte* src, uint len) {
    // Encode the First Frame

    // frame type = 1 (first nibble)
    // length is in the following 3 nibbles
    buff[0] = 0x10 | ((len >> 8) & 0x0f);
    buff[1] = len & 0xff;
    memcpy(buff+2, src, MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH);
    framesStored = 1;

    // move on to encode the consecutive frames
    uint bytesEncoded = MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH;
    uint seqNo = 1;
    while (bytesEncoded < len) {
        byte*   newFrame = GetFramePtr(framesStored);
        newFrame[0] = 0x20 | (seqNo & 0x0f);

        uint nextFramePayloadLength = (len - bytesEncoded);
        if (MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH < nextFramePayloadLength)
            nextFramePayloadLength = MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH;
        memcpy(newFrame+1, src+bytesEncoded, nextFramePayloadLength);

        ++seqNo;
        ++framesStored;
        bytesEncoded += nextFramePayloadLength;
    }

    bytesUsed = framesStored * CANBUS_FRAME_SIZE;
    return bytesUsed;
}

byte* CanBusBuffer::GetFramePtr(uint frameNumber) {
    return (buff + frameNumber * CANBUS_FRAME_SIZE);
}

void CanBusBuffer::Show() {
    printf("CanBusBuffer @ %p:\n", this);
    printf("  bufferOverrun : %s\n", (bufferOverrun ? "true" : "false"));
    printf("  framesStored  : %u\n", framesStored);
    printf("  bytesUsed     : %u\n", bytesUsed);
    for (uint frameNo = 0; frameNo < framesStored; ++frameNo) {
        byte* frameBase = GetFramePtr(frameNo);
        printf("  frame %3u :", frameNo);
        for (uint i = 0; i < CANBUS_FRAME_SIZE; ++i)
            printf(" %02x", frameBase[i]);
        printf("\n");
    }
}
// eof