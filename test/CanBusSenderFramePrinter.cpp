#include <stdio.h>

#include "CanBusSenderFramePrinter.h"

void CanBusSenderFramePrinter::SendFrame(byte* src, uint len) {
    printf(" > SendFrame:");
    for (uint i = 0; i < len; ++i)
        printf(" %02x", src[i]);
    printf("\n");
}

