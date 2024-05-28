#ifndef LLOPEN
#define LLOPEN

#include "linklayer.h"
#include <fcntl.h>
#include <sys/types.h>

#define buff_size 256

int llopen(linkLayer connectionParameters);

void set_termios(int fd, struct termios* tio, struct linkLayer params);

void* create_conn_message(enum SIGNALS type, int role, char* buff, int length);

int send_message(int fd, enum SIGNALS type, char* buff, int legnth, int role);

int oread_buffer(int fd, char* buff, int length);

int check_for_SET(int fd, int role);

int check_for_UA(int fd, int role);

#endif