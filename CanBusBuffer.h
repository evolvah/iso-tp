#ifndef CanBusBuffer_h_
#define CanBusBuffer_h_

#include "Utils.h"

// Objects of this class are capable of holding an entire "long" application message
// fragmented into a number of 8-byte CANbus frames. When a complete message is buffered,
// it can be retrieved at once.
class CanBusBuffer
{
    static const uint   MAX_APPLICATION_MESSAGE_LENGTH              = 512;
    static const uint   MAX_ISOTP_SINGLE_FRAME_PAYLOAD_LENGTH       = 7;
    static const uint   MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH        = 6;
    static const uint   MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH  = 7;
    static const uint   CANBUS_FRAME_SIZE                           = 8;

    // The constant of "2" accounts for rounding down of the division and then
    // adds another frame for the first 6-byte frame
    static const uint   MAX_ISOTP_BUFFER_LENGTH =
        ((MAX_APPLICATION_MESSAGE_LENGTH - MAX_ISOTP_FIRST_FRAME_PAYLOAD_LENGTH) /
            MAX_ISOTP_CONSECUTIVE_FRAME_PAYLOAD_LENGTH + 2) * CANBUS_FRAME_SIZE;

    // The actual buffer
    byte                buff[MAX_ISOTP_BUFFER_LENGTH];
    // The message was not fetched before the next one arrived
    bool                bufferOverrun;

    uint                bytesUsed;
    uint                framesStored;

    // Helper method to encode long messages
    uint                EncodeMultiFrameMessage(byte* src, uint len);
public:
                CanBusBuffer();
                ~CanBusBuffer();

    // If message is available in the buffer, safely extract it into the provided
    // memory region, not to exceed the specified byte count.
    // Return the number of bytes extracted or 0 if the message is not ready yet.
    uint        FetchMessage(byte* dst, uint maxBytes);

    // Encode the byte array into a seqence of CAN frames in the internal buffer.
    // Returns the number of bytes used to encode the message.
    uint        EncodeMessage(byte* src, uint len);

    // Get a pointer to a first byte of a frame
    byte*       GetFramePtr(uint frameNumber);

    // Clears the buffer and associated errors
    void        Clear();

    // Prints the object to the console
    void        Show();
};

#endif

