#include "../include/llopen.h"

int main(int argc, char** argv)
{
    linkLayer l = {
        .serialPort = "/dev/ttyS11",
        .role = 1,
        .baudRate = B38400,
        .numTries = 4,
        .timeOut = 100
    };

    return llopen(l);
}