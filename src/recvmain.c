#include "../include/llopen.h"
#include "../include/llread.h"
#include "../include/llclose.h"


int main(int argc, char** argv)
{
    linkLayer l = {
        .serialPort = "/dev/ttyS11",
        .role = 1,
        .baudRate = B38400,
        .numTries = 4,
        .timeOut = 500
    };

    char* buffer;

    llopen(l);

    llread(buffer);

    llclose(l, 0);

    return 1;
}