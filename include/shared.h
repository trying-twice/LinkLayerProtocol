#ifndef SHARED
#define SHARED

#include "linklayer.h"
#include <time.h>

typedef struct packet
{
    char* buffer;
    int size;
    char* last;
    char* first;
}   packet;

int append_to_packet(struct packet* packet, char* buffer, size_t buff_length);

bool clean_packet(struct packet* packet);

int read_buffer(int fd, char* buff, int length);

int validate_packet_BCC1(int fd, struct packet* packet, enum SIGNALS* type, int role, int retr_count);

bool construct_BCC1(enum SIGNALS type, int role, char* buff, int length);

char calculate_BCC2(char* data, int data_length);

#endif