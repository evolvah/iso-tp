#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned char   byte;
typedef unsigned int    uint;

class CanBusSender
{
public:
    CanBusSender() {};
    ~CanBusSender() {};
    
    void SendFrame(byte* data, uint len) {
    };
};

class CanBusFrame {
public:
    static const uint   MAX_SINGLE_FRAME        = 7;
    static const uint   MAX_FIRST_FRAME         = 6;
    static const uint   MAX_CONSECUTIVE_FRAME   = 7;
    static const uint   MAX_FRAME_PAYLOAD       = 8;

private:
    CanBusFrame(const CanBusFrame& rhs);
    CanBusFrame& operator= (const CanBusFrame& rhs);

    byte    buff[MAX_FRAME_PAYLOAD];

public:
    CanBusFrame() {};

    void Reset() { memset(buff, 0, MAX_FRAME_PAYLOAD); }

    void DecorateAsSingleFrame(byte* dataAddr, uint dataLen) {
        uint bytesToStore = (dataLen < MAX_SINGLE_FRAME ? dataLen : MAX_SINGLE_FRAME);
        Reset();
        buff[0] = 0x00 | (dataLen & 0x0f);
        memcpy(buff+1, dataAddr, bytesToStore);
    }

    void DecorateAsFirstFrame(byte* dataAddr, uint dataLen) {
        uint bytesToStore = (dataLen < MAX_FIRST_FRAME ? dataLen : MAX_FIRST_FRAME);
        Reset();
        buff[0] = 0x10 | ((dataLen >> 8) & 0x0f);
        buff[1] = dataLen & 0xff;
        memcpy(buff+2, dataAddr, bytesToStore);
    }

    void DecorateAsConsecutiveFrame(byte* dataAddr, uint dataLen, uint seqNo) {
        uint bytesToStore = (dataLen < MAX_CONSECUTIVE_FRAME ? dataLen : MAX_CONSECUTIVE_FRAME);
        Reset();
        buff[0] = 0x20 | (seqNo & 0x0f);
        memcpy(buff+1, dataAddr, bytesToStore);
    }

    void SendViaSender(CanBusSender& sender) {
        Print();
        sender.SendFrame(buff, MAX_FRAME_PAYLOAD);
    }

    void Print() const {
        printf("CanBusFrame @ %p :", this);
        for (uint i = 0; i < MAX_FRAME_PAYLOAD; ++i)
            printf(" %02x", buff[i]);
        printf("\n");
    }
};

class IsoTp
{
    CanBusFrame frame;

    void SendSingleFrame(byte* dataAddr, uint dataLen, CanBusSender& sender) {
        frame.DecorateAsSingleFrame(dataAddr, dataLen);
        frame.SendViaSender(sender);
    }

    void SendFirstFrame(byte* dataAddr, uint dataLen, CanBusSender& sender) {
        frame.DecorateAsFirstFrame(dataAddr, dataLen);
        frame.SendViaSender(sender);
    }

    void SendConsecutiveFrame(byte* dataAddr, uint dataLen, CanBusSender& sender) {
        frame.DecorateAsConsecutiveFrame(dataAddr, dataLen, 1);
        frame.SendViaSender(sender);
    }

public:
    IsoTp() {};
    ~IsoTp() {};
    void SendMessage(byte* dataAddr, uint dataLen, CanBusSender& sender) {
        if (dataLen > CanBusFrame::MAX_SINGLE_FRAME) {
            // Multi Frame mode
            uint pos = 0;
            SendFirstFrame(dataAddr, CanBusFrame::MAX_FIRST_FRAME, sender);
            pos += CanBusFrame::MAX_FIRST_FRAME;

            uint bytesLeft = dataLen - pos;
            while (bytesLeft > CanBusFrame::MAX_CONSECUTIVE_FRAME) {
                SendConsecutiveFrame(dataAddr+pos, CanBusFrame::MAX_CONSECUTIVE_FRAME, sender);
                bytesLeft -= CanBusFrame::MAX_CONSECUTIVE_FRAME;
                pos += CanBusFrame::MAX_CONSECUTIVE_FRAME;
            }

            if (bytesLeft > 0) {
                SendConsecutiveFrame(dataAddr+pos, bytesLeft, sender);
                pos += bytesLeft;
                bytesLeft -= bytesLeft;                
            }
        } else {
            // Single Frame mode
            SendSingleFrame(dataAddr, dataLen, sender);
        }
    }
};


int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Bad argument!\n");
        return 1;
    }

    uint len = atoi(argv[1]);
    if (len == 0)
        len = 10;

    if (len > 600)
        len = 512;

    printf("Working with a buffer of %u bytes\n", len);

    byte* data = new byte [len];
    for (uint i = 0; i < len; ++i)
        data[i] = i & 0xff;

    IsoTp           isoTpApi;
    CanBusSender    canSender;

    isoTpApi.SendMessage(data, len, canSender);

    delete [] data;
    return 0;
}
