#ifndef LINKLAYER
#define LINKLAYER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

typedef struct linkLayer
{
    char serialPort[50];
    int role;               //defines the role of the program: 0 == Transmitter, 1 == Receiver
    int baudRate;
    int numTries;
    int timeOut;
} linkLayer;

enum SIGNALS
{
    FLAG = '\x5c',
    ESCAPE = '\x5d',
    SIGNAL_FLAG = '\x7c',
    SIGNAL_ESCAPE = '\x7d',
    TRANS_ADDR = '\x03',
    RECV_ADDR = '\x01',
    SET = '\x08',
    UA = '\x06',
    DISC = '\x0A',
    I1 = '\xC0',
    I0 = '\x80',
    RR1 = '\x11',
    RR0 = '\x01',
    REJ1 = '\x15',
    REJ = '\x05',
    ERROR = '\x00'
};

//ROLE
#define NOT_DEFINED -1
#define TRANSMITTER 0
#define RECEIVER 1


//SIZE of maximum acceptable payload; maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000
#define BCC1_FIELD_LENGTH 4

//CONNECTION deafault values
#define BAUDRATE_DEFAULT B38400
#define MAX_RETRANSMISSIONS_DEFAULT 3
#define TIMEOUT_DEFAULT 4
#define _POSIX_SOURCE 1 /* POSIX compliant source */

//MISC
#define FALSE 0
#define TRUE 1

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

/*
// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters);
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize);
// Receive data in packet
int llread(char* packet);
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics);
*/
#endif

/*THREE TYPES OF FRAMES:
    I: DATA FRAMES : F A C BCC1 DATA ... DATA BCC2 F
    S:
        -> F A C BCC1 F
    U:
*/