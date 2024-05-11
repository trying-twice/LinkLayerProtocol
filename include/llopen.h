#ifndef LLOPEN
#define LLOPEN

#include "linklayer.h"
#include <fcntl.h>
#include <sys/types.h>

#define ack_message_length 5
#define buff_size 256

void set_termios(int fd, struct termios* tio, struct linkLayer params);

void* create_conn_message(enum SIGNALS type, int role, char* buff, int length);

int send_message(int fd, enum SIGNALS type, char* buff, int legnth, int role, struct flock* lock);

int read_buffer(int fd, char* buff, int length, struct flock* lock);

int check_for_SET(int fd, struct flock* lock, int role);

int check_for_UA(int fd, struct flock* lock, int role);

#endif