#ifndef IsoTpBuffer_h_
#define IsoTpBuffer_h_

#include "Utils.h"
#include "CanBusSender.h"

// Objects of this class are capable of holding an entire "long" application message
// fragmented into a number of 8-byte CANbus frames. When a complete message is buffered,
// it can be retrieved at once.
class IsoTpBuffer
{
    static const uint   MAX_APPLICATION_MESSAGE_LENGTH              = 512;
    static const uint   MAX_ISOTP_SINGLE_FRAME_PAYLOAD_LENGTH       = 7;
    static const uint   MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH        = 6;
    static const uint   MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH  = 7;
    static const uint   CANBUS_FRAME_SIZE                           = 8;

    // The actual buffers
    byte                txBuff[CANBUS_FRAME_SIZE];
    byte                rxBuff[MAX_APPLICATION_MESSAGE_LENGTH];

    // The message was not fetched before the next one arrived
    bool                bufferOverrun;
    uint                rxBuffPosition;
    uint                rxFrameCount;
    uint                rxBytesToReceive;

    // Helper method to encode long messages
    uint                EncodeMultiFrameMessage(byte* src, uint len, CanBusSender& sender);

    bool                ProcessRxSingleFrame(byte* src);
    bool                ProcessRxFirstFrame(byte* src);
    bool                ProcessRxConsecutiveFrame(byte* src);
    bool                ProcessRxFlowControlFrame(byte* src);

public:
                IsoTpBuffer();
                ~IsoTpBuffer();

    // If message is available in the buffer, safely extract it into the provided
    // memory region, not to exceed the specified byte count.
    // Return the number of bytes extracted or 0 if the message is not ready yet.
    uint        FetchMessage(byte* dst, uint maxBytes);

    // Encode the byte array into a seqence of CAN frames in the internal buffer.
    // Returns 0 if success.
    uint        TransmitMessage(byte* src, uint len, CanBusSender& sender);

    // Process a newly received CAN bus frame. This function is invoked from
    // an interrupt handler. Proper care should be taken.
    bool        ProcessRxFrame(byte* src, uint len);

    // Clears the buffer and associated errors
    void        Clear();

    // Print the object to the console in multiple ways
    void        Show();
    void        ShowRxBuffer();
};

#endif

