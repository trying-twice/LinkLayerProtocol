#ifndef LLOPEN
#define LLOPEN

#include <termios.h>
#include "linklayer.h"

#define ack_message_length 5
#define buff_size 256


void set_termios(struct termios* tio, struct linkLayer params);

void* create_conn_message(enum SIGNALS type, int role, char* buff);

int send_message(int fd, enum SIGNALS type, char* buff, int role);

void read_buffer(int fd, char* buff, int length);

int check_for_UA(int fd, char *buff, int length);

int check_for_SET(int fd, char *buff, int length);

#endif