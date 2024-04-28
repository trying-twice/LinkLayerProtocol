#include "../include/llopen.h"

int llopen(linkLayer connectionParameters)
{

    int fd;
    struct termios oldtio, newtio;
    bzero(&newtio, sizeof(newtio));

    char* s_port = connectionParameters.serialPort;
    fd = open(s_port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(s_port);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1) /* preserve current port settings */
    {
        perror("tcgetattr");
        exit(-1);
    }

    set_termios(fd, &newtio, connectionParameters);

    tcflush(fd, TCIOFLUSH);        /* Flush the fd to get rid of old data */

    printf("New termios structure set\n");
    int role = connectionParameters.role;
    char buff[ack_message_length];
    bzero(buff, ack_message_length);

    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if(!role)
    {   
        printf("Transmitter!\n");
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            res = send_message(fd, SET, buff, ack_message_length, role, &lock);
            usleep(connectionParameters.timeOut*1000);
            printf("buff after send - %x%x%x%x\n", buff[0], buff[1], buff[2], buff[3]);
            if(check_for_UA(fd, buff, ack_message_length, &lock)) break;
        }
        printf("buff after read - %x%x%x%x\n", buff[0], buff[1], buff[2], buff[3]);
    }
    else
    {
        printf("Receiver!\n");
        int res = 0;
        for (int i = 0; ; i++)
        {
            usleep(connectionParameters.timeOut*1000);
            if(check_for_SET(fd, buff, ack_message_length, &lock)) break;
        }
        printf("buff after read - %x%x%x%x\n", buff[0], buff[1], buff[2], buff[3]);
        res = send_message(fd, UA, buff, ack_message_length, role, &lock);
        printf("buff after send - %x%x%x%x\n", buff[0], buff[1], buff[2], buff[3]);
    }

    return 1;
}

void set_termios(int fd, struct termios* tio, struct linkLayer params)
{
    tio->c_cflag = params.baudRate | CS8 | CLOCAL | CREAD;
    tio->c_iflag = IGNPAR;
    tio->c_oflag = 0;
    tio->c_lflag = 0;                       /* set input mode NON-CANONICAL*/
    tio->c_cc[VTIME] = params.timeOut;      /* inter-character timer unused,  waits 0 seconds between chars*/
    tio->c_cc[VMIN] = params.numTries;      /* blocking read until x chars received */

    if (tcsetattr(fd, TCSANOW, tio) == -1) {perror("tcsetattr"); exit(-1);}
}

void* create_conn_message(enum SIGNALS type, int role, char* buff, int length)
{
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char cntrl = type;
    char BCC1 = addr ^ cntrl;

    if(length >= ack_message_length)
    {
        bzero(buff, ack_message_length);
        buff[0] = FLAG;
        buff[1] = addr;
        buff[2] = cntrl;
        buff[3] = BCC1;
        buff[4] = FLAG;

        //printf(" HERE - %x%x%x%x\n", buff[0], buff[1], buff[2], buff[3]);
    }else { perror("create_conn_message"); exit(-1); }
}

int send_message(int fd, enum SIGNALS type, char* buff, int length, int role, struct flock* lock)
{
    int res;
    create_conn_message(type, role, buff, length);

    lock->l_type = F_WRLCK;
    if(fcntl(fd, F_SETLK, lock) < 0) perror("SEND_LOCK");
    
    //printf("b - %c <-\n", buff[2]);
    res = write(fd, buff, ack_message_length);
    printf("WRITE DONE!\n");

    lock->l_type = F_UNLCK;
    fcntl(fd, F_SETLK, lock);

    return res;
}

int read_buffer(int fd, char* buff, int length, struct flock* lock)
{
    int total_read = 0;
    int res = -1;

    lock->l_type = F_WRLCK;
    if(fcntl(fd, F_SETLK, lock) < 0) perror("READ_LOCK");

    while (total_read < length)
    {
        res = read(fd, buff + total_read, length - total_read); //only reasd length chars
        printf("READ DONE!\n");
        //printf("%d\n", res);
        if (res < 0) {perror("read"); break;}
        total_read += res;
    }

    //if (ftruncate(fd, 0) < 0) perror("ftruncate");  // wipe the file --> tcflush?

    lock->l_type = F_UNLCK;
    fcntl(fd, F_SETLK, lock);
    
    buff[total_read] = '\0';
    return res;
}

int check_for_UA(int fd, char *buff, int length, struct flock* lock)
{
    //bzero(buff, length);
    read_buffer(fd, buff, length, lock);
    return buff[2] == 0x06;
}
 
int check_for_SET(int fd, char *buff, int length, struct flock* lock)
{
    //bzero(buff, length);
    read_buffer(fd, buff, length, lock);
    return buff[2] == 0x08;
}