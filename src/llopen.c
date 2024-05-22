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
    char buff[BCC1_FIELD_LENGTH];
    bzero(buff, BCC1_FIELD_LENGTH);

    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if(!role)
    {   
        printf("Transmitter!\n");
        int res = 0;
        res = send_message(fd, SET, buff, BCC1_FIELD_LENGTH, role);
        printf("SET sent!\n");
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            usleep(connectionParameters.timeOut*1000);
            if(check_for_UA(fd, role)) break;
        }
        printf("UA received!\n");
    }
    else
    {
        printf("Receiver!\n");
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            if(check_for_SET(fd, role)) break;
            usleep(connectionParameters.timeOut*1000);
        }
        printf("SET received!\n");
        res = send_message(fd, UA, buff, BCC1_FIELD_LENGTH, role);
        printf("UA sent!\n");
    }

    return 1;
}

void set_termios(int fd, struct termios* tio, struct linkLayer params)
{
    tio->c_cflag = params.baudRate | CS8 | CLOCAL | CREAD;
    tio->c_iflag = IGNPAR;
    tio->c_oflag = 0;
    tio->c_lflag = 0;                       /* set input mode NON-CANONICAL*/
    tio->c_cc[VTIME] = 0.1;      /* inter-character timer unused,  waits 0 seconds between chars*/
    tio->c_cc[VMIN] = 0;      /* blocking read until x chars received */

    if (tcsetattr(fd, TCSANOW, tio) == -1) {perror("tcsetattr"); exit(-1);}
}

void* create_conn_message(enum SIGNALS type, int role, char* buff, int length)
{
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char cntrl = type;
    char BCC1 = addr ^ cntrl;

    if(length >= BCC1_FIELD_LENGTH)
    {
        bzero(buff, BCC1_FIELD_LENGTH);
        buff[0] = FLAG;
        buff[1] = addr;
        buff[2] = cntrl;
        buff[3] = BCC1;

    } else { perror("create_conn_message"); exit(-1); }
}

int send_message(int fd, enum SIGNALS type, char* buff, int length, int role)
{
    int res;
    create_conn_message(type, role, buff, length);

    res = write(fd, buff, BCC1_FIELD_LENGTH);

    return res; 
}

int oread_buffer(int fd, char* buff, int length)
{
    int total_read = 0;
    int res = 1;

    while (total_read < length && res > 0)
    {
        res = read(fd, buff + total_read, length - total_read); //only reasd length chars
        if (res < 0) {perror("read"); break;}
        total_read += res;
    }
    
    return total_read;
}

int check_for_SET(int fd, int role)
{
    int res = 0;
    char c_read = ' ';
    int n_pass = TRUE;
    int ret = 0;
    char BCC1 = TRANS_ADDR ^ SET;
    int flag = -1;
    int max_zero_attemps = 150;
    for (int state = 0, it = 0; state < BCC1_FIELD_LENGTH; state++)
    {
        n_pass = TRUE;
        while(res >= 0 && n_pass)   // infinitte loops
        {
            res = oread_buffer(fd, &c_read, 1);
            it = res == 0 ? it + 1: 0;
            if (!res) continue;
            switch (state)
            {
            case 0: 
                //printf("read 0 - %x\n", c_read);
                if (c_read == FLAG) {n_pass = FALSE;}
                break;
            case 1:
                //printf("read 1 - %x \n", c_read);
                if (c_read == TRANS_ADDR) {n_pass = FALSE;}
                else if (c_read == FLAG) continue;
                else state = 0;
                break;
            case 2:
                //printf("read 2 - %x \n", c_read);
                if (c_read == SET) {n_pass = FALSE; ret = 1;}
                else if (c_read == FLAG) state = 1;
                else state = 0;
                break;
            case 3:
                //printf("read 3 - %x \n", c_read);
                if (c_read == BCC1) {n_pass = FALSE;}
                else if (c_read == FLAG) {state = 1; if(ret) ret = 0;}
                else {state = 0; if(ret) ret = 0;}
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

int check_for_UA(int fd, int role)
{
    int res = 0;
    char c_read = ' ';
    int n_pass = TRUE;
    int ret = 0;
    char BCC1 = RECV_ADDR ^ UA;
    int flag = -1;
    int max_zero_attemps = 150;
    for (int state = 0, it = 0; state < BCC1_FIELD_LENGTH; state++)
    {
        n_pass = TRUE;
        while(res >= 0 && n_pass)   // infinitte loops
        {
            res = oread_buffer(fd, &c_read, 1);
            it = res == 0 ? it + 1: 0;
            if (!res) continue;
            switch (state)
            {
            case 0: 
                //printf("read 0 - %x\n", c_read);
                if (c_read == FLAG) {n_pass = FALSE;}
                break;
            case 1:
                //printf("read 1 - %x \n", c_read);
                if (c_read == RECV_ADDR) {n_pass = FALSE;}
                else if (c_read == FLAG) continue;
                else state = 0;
                break;
            case 2:
                //printf("read 2 - %x \n", c_read);
                if (c_read == UA) {n_pass = FALSE; ret = 1;}
                else if (c_read == FLAG) state = 1;
                else state = 0;
                break;
            case 3:
                //printf("read 3 - %x \n", c_read);
                if (c_read == BCC1) {n_pass = FALSE;}
                else if (c_read == FLAG) {state = 1; if(ret) ret = 0;}
                else {state = 0; if(ret) ret = 0;}
                break;
            default:
                break;
            }
        }
    }
    return ret;
}
