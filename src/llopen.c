#include "../include/linklayer.h"
#include <termios.h>

#define ack_message_length 5

void set_termios(struct termios* tio, struct linkLayer params);
char* create_conn_message(int role, char* buff);
void wait_conn_response(int fd);
char check_message_type(char* buff);

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
    strcpy(buff, create_conn_message(role, buff));

    if(!role)
    { 
        int res = write(fd, buff, ack_message_length);
        wait_response();
    }
    else
    {
        wait_response();
        int res = write(fd, buff, ack_message_length);
    }


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

char* create_conn_message(int role, char* buff)
{
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char cntrl = role ? UA : SET;
    char BCC1 = addr ^ cntrl;

    if(sizeof(buff)/sizeof(buff[0]) >= 5)
    {
        buff[0] = FLAG;
        buff[1] = addr;
        buff[2] = cntrl;
        buff[3] = BCC1;
        buff[4] = FLAG;
    }else { perror("create_conn_message"); exit(-1); }

    return buff;
}

void wait_conn_response(int fd)
{
    char buff[64];
    int res = 0;
    while (res < 5)
    {
        int res = read(fd, buff, 5);   /* returns after 5 chars have been input */
        buff[res] = 0;                    /* so we can printf... */
        if (res >= 5)
        {
            printf(":%x%x%x%x%x:%d\n", buff[0], buff[1], buff[2], buff[3], buff[4], res);
            if(buff[2] == UA) break;
        }
    }
}
