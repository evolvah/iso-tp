#include <stdlib.h>
#include <stdio.h>

#include "CanBusBuffer.h"

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

    CanBusBuffer cbb;

    cbb.EncodeMessage(data, testByteCount);

    cbb.Show();

    return 0;
}