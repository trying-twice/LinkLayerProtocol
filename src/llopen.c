#include "../include/llopen.h"

int llopen(linkLayer connectionParameters)
{

    int fd;
    struct termios oldtio, newtio;
    bzero(&newtio, sizeof(newtio));

    char* s_port = connectionParameters.serialPort;
    fd = open(s_port, O_RDWR | O_NOCTTY );
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

    set_termios(&newtio, connectionParameters);

    tcflush(fd, TCIOFLUSH);         /* Flush the fd to get rid of old data */


    printf("New termios structure set\n");

    int role = connectionParameters.role;
    char buff[ack_message_length];

    if(!role)
    {   
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            res = send_message(fd, SET, buff, role);
            usleep(connectionParameters.timeOut*1000);
            if(check_for_UA(fd, buff, ack_message_length)) break; 
        }
    }
    else
    {
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries + 10; i++)
        {
            usleep(connectionParameters.timeOut*1000);
            printf("ada\n");
            if(check_for_SET(fd, buff, ack_message_length)) {printf("ada\n"); printf("%s", buff); break;}
        }
        res = send_message(fd, UA, buff, role);
    }

    return 1;
}

void set_termios(struct termios* tio, struct linkLayer params)
{
    tio->c_cflag = params.baudRate | CS8 | CLOCAL | CREAD;
    tio->c_iflag = IGNPAR;
    tio->c_oflag = 0;
    tio->c_lflag = 0;                       /* set input mode NON-CANONICAL*/
    tio->c_cc[VTIME] = params.timeOut;      /* inter-character timer unused,  waits 0 seconds between chars*/
    tio->c_cc[VMIN] = params.numTries;      /* blocking read until x chars received */
}

void* create_conn_message(enum SIGNALS type, int role, char* buff)
{
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char cntrl = type;
    char BCC1 = addr ^ cntrl;

    if(sizeof(buff)/sizeof(buff[0]) >= 5)
    {
        buff[0] = FLAG;
        buff[1] = addr;
        buff[2] = cntrl;
        buff[3] = BCC1;
        buff[4] = FLAG;

        printf(" HERE - %x%x%x%x", buff[0], buff[1], buff[2], buff[3]);
    }else { perror("create_conn_message"); exit(-1); }
}

int send_message(int fd, enum SIGNALS type, char* buff, int role)
{
    char response[buff_size];
    int res;
    buff = create_conn_message(type, role, buff);
    res = write(fd, buff, ack_message_length);

    return res;
}

void read_buffer(int fd, char* buff, int length)
{
    int total_read = 0;
    while (total_read < length)
    {
        int res = read(fd, buff + total_read, length - total_read); //only read length chars
        if (res < 0) {perror("read"); break;}
        //else if (res == 0) break; //EOF
        total_read += res;
    }
    buff[total_read] = '\0'; // Null-terminate the buffer
}

int check_for_UA(int fd, char *buff, int length)
{
    bzero(buff, length);
    read_buffer(fd, buff, length);
    printf("%x", buff[2]);
    if(buff[2] == 0x06) printf("!#!\n");
    return buff[2] == 0x06;
}

int check_for_SET(int fd, char *buff, int length)
{
    bzero(buff, length);
    read_buffer(fd, buff, length);
    printf("%x", buff[2]);
    if(buff[2] == 0x08) printf("!#!dad\n");
    return buff[2] == 0x08;
}