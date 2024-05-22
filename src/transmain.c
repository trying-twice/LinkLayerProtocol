#include "../include/llopen.h"
#include "../include/llwrite.h"
#include "../include/llclose.h"

int main(int argc, char** argv)
{
    linkLayer l = {
        .serialPort = "/dev/ttyS10",
        .role = 0,
        .baudRate = B38400,
        .numTries = 4,
        .timeOut = 500
    };

    char buffer[100] = {0};
    int s = 100;

    llopen(l);

    llwrite(buffer, s);

    llclose(l, 0);

    return 1;
}