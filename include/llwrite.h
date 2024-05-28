#ifndef LLWRITE
#define LLWRITE

#include "shared.h" 

int llwrite(char* buffer, int length);

int send_packet(int fd, struct packet* packet, char type, char* data, int data_length);

bool construct_packet(struct packet* packet, char type, char* data, int data_length);

void mod_buff(char* buffer, int length);

bool construct_BCC2(char* data, int data_length, char* dest);

enum SIGNALS read_response(int fd);

#endif