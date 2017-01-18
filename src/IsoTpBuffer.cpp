#include <string.h>
#include <stdio.h>

#include "IsoTpBuffer.h"

IsoTpBuffer::IsoTpBuffer() {
    Clear();
}

IsoTpBuffer::~IsoTpBuffer() {
}

void IsoTpBuffer::Clear() {
    memset(txBuff, 0, CANBUS_FRAME_SIZE);
    memset(rxBuff, 0, MAX_APPLICATION_MESSAGE_LENGTH);
    bufferOverrun = false;
    rxBuffPosition = 0;
    rxBytesToReceive = 0;
    rxFrameCount = 0;
}

uint IsoTpBuffer::FetchMessage(byte* dst, uint maxBytes) {
    // TODO: need to prevent ProcessRxFrame execution somehow
    uint bytesToCopy = (maxBytes < rxBuffPosition ? maxBytes : rxBuffPosition);
    memcpy(dst, rxBuff, bytesToCopy);

    bufferOverrun = false;
    rxBuffPosition = 0;
    rxBytesToReceive = 0;
    rxFrameCount = 0;

    return bytesToCopy;
}

uint IsoTpBuffer::TransmitMessage(byte* src, uint len, CanBusSender& sender) {
    // Let's do a sanity check first
    if (len == 0 || len > MAX_APPLICATION_MESSAGE_LENGTH)
        return 1;

    if (len > MAX_ISOTP_SINGLE_FRAME_PAYLOAD_LENGTH)
        return EncodeMultiFrameMessage(src, len, sender);

    // Encode as a Single Frame
    memset(txBuff, 0, CANBUS_FRAME_SIZE);
    txBuff[0] = len;
    memcpy(txBuff+1, src, len);
    sender.SendFrame(txBuff, CANBUS_FRAME_SIZE);

    return 0;
}

/* private */
uint IsoTpBuffer::EncodeMultiFrameMessage(byte* src, uint len, CanBusSender& sender) {
    // Encode the First Frame

    // frame type = 1 (first nibble)
    // length is in the following 3 nibbles
    txBuff[0] = 0x10 | ((len >> 8) & 0x0f);
    txBuff[1] = len & 0xff;
    memcpy(txBuff+2, src, MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH);
    sender.SendFrame(txBuff, CANBUS_FRAME_SIZE);

    // move on to encode the consecutive frames
    uint bytesEncoded = MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH;
    uint seqNo = 1;
    while (bytesEncoded < len) {
        memset(txBuff, 0, CANBUS_FRAME_SIZE);
        txBuff[0] = 0x20 | (seqNo & 0x0f);

        uint nextFramePayloadLength = (len - bytesEncoded);
        if (MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH < nextFramePayloadLength)
            nextFramePayloadLength = MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH;
        memcpy(txBuff+1, src+bytesEncoded, nextFramePayloadLength);
        sender.SendFrame(txBuff, CANBUS_FRAME_SIZE);

        ++seqNo;
        bytesEncoded += nextFramePayloadLength;
    }

    return 0;
}

bool IsoTpBuffer::ProcessRxFrame(byte* src, uint len) {
    // Check frame size
    if (len != CANBUS_FRAME_SIZE)
        return false;

    bool retVal = true;
    uint frameType = src[0] >> 4;
    switch (frameType) {
        case 0: // SINGLE FRAME
            retVal = ProcessRxSingleFrame(src);
            break;
        case 1: // FIRST FRAME of multi-frame sequence
            retVal = ProcessRxFirstFrame(src);
            break;
        case 2: // CONSECUTIVE FRAME of multi-frame sequence
            retVal = ProcessRxConsecutiveFrame(src);
            break;
        case 3: // FLOW CONTROL
            retVal = ProcessRxFlowControlFrame(src);
            break;
        default:
            retVal = false;
            break;
    }

    return retVal;
}

/* private */
bool IsoTpBuffer::ProcessRxSingleFrame(byte* src) {
    if (rxBuffPosition != 0) {
        bufferOverrun = true;
        return false;
    }

    rxBytesToReceive = 0;
    rxBuffPosition = src[0] & 0x0f;
    memcpy(rxBuff, src+1, rxBuffPosition);
    return true;
}

/* private */
bool IsoTpBuffer::ProcessRxFirstFrame(byte* src) {
    if (rxBuffPosition != 0) {
        bufferOverrun = true;
        return false;
    }

    rxBytesToReceive = ((src[0] & 0x0f) << 8) | src[1];
    memcpy(rxBuff+rxBuffPosition, src+2, MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH);
    rxBuffPosition = MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH;
    rxBytesToReceive -= MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH;
    rxFrameCount = 1; 
    return true;
}

/* private */
bool IsoTpBuffer::ProcessRxConsecutiveFrame(byte* src) {
    // Bail immediately if we are in the overrun state
    if (bufferOverrun) return false;

    // Fail if we have not recieved the previous frame
    uint currFrameSeqNo = src[0] & 0x0f;
    if ((rxFrameCount & 0x0f) != currFrameSeqNo) return false;

    // Are we trying to process extra frames, when the message was completely transferred?
    if (rxBytesToReceive == 0) return false;

    uint bytesToReadThisTime = MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH;
    if (rxBytesToReceive < bytesToReadThisTime)
        bytesToReadThisTime = rxBytesToReceive;

    memcpy(rxBuff+rxBuffPosition, src+1, bytesToReadThisTime);
    rxBuffPosition += bytesToReadThisTime;
    rxBytesToReceive -= bytesToReadThisTime;
    ++rxFrameCount;

    return true;
}

/* private */
bool IsoTpBuffer::ProcessRxFlowControlFrame(byte* src) {
    return true;
}

void IsoTpBuffer::Show() {
    printf("IsoTpBuffer @ %p:\n", this);
    printf("  bufferOverrun : %s\n", (bufferOverrun ? "true" : "false"));
}

void IsoTpBuffer::ShowRxBuffer() {
    printf("IsoTpBuffer @ %p:\n", this);
    printf("  bufferOverrun  : %s\n", (bufferOverrun ? "true" : "false"));
    printf("  rxBuffPosition : %d\n", rxBuffPosition);
    for (uint i = 0; i < rxBuffPosition; ++i) {
        if (i % 8 == 0) printf("\n%3u:", i);
        printf(" %02x", rxBuff[i]);
    }
    printf("\n");
}


// eof