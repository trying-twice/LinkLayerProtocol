#ifndef LLREAD
#define LLREAD

#include "shared.h"

int llread(char* buffer);

enum SIGNALS get_packet(int fd, struct packet* packet, int* read);

int validate_packet_BCC2(int fd, struct packet* packet, enum SIGNALS* type, int retr_count);

bool send_response(int fd, enum SIGNALS type);

void packet_to_buffer(struct packet* packet, char* buffer);

#endif