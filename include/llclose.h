#ifndef LLCLOSE
#define LLCLOSE

#include "linklayer.h"
#include "llclose.h"
#include "llopen.h"
#include <fcntl.h>
#include <sys/types.h>

int llclose(linkLayer connectionParameters, int showStatistics);

int check_for_DISC(int fd, int role);

int check_for_UA2(int fd, int role);

#endif