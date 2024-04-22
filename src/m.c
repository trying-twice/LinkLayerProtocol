#include "../include/llopen.h"

int main(int argc, char** argv)
{
    linkLayer l = {
        .serialPort = "/dev/ttyS11",
        .role = 1,
        .baudRate = B38400,
        .numTries = 4,
        .timeOut = 0
    };

    return llopen(l);
}