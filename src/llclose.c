#include "../include/llclose.h"

int llclose(linkLayer connectionParameters, int showStatistics)
{
    char* s_port = connectionParameters.serialPort;
    int fd = open(s_port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(s_port);
        exit(-1);
    }

    int role = connectionParameters.role;
    char buff[BCC1_FIELD_LENGTH];

    if(!role)
    {   
        printf("Transmitter!\n");
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            res = send_message(fd, DISC, buff, BCC1_FIELD_LENGTH, role);
            printf("DISC sent! num of try : %d\n", i);
            usleep(connectionParameters.timeOut*1000);
            if(check_for_DISC(fd, role)) break;
        }
        printf("DISC received!\n");
        res = send_message(fd, UA, buff, BCC1_FIELD_LENGTH, role);
        if(res < 0) return -1;
    }
    else
    {
        printf("Receiver!\n");
        int res = 0;
        for (int i = 0; i < connectionParameters.numTries; i++)
        {
            if(check_for_DISC(fd, role)) break;
            usleep(connectionParameters.timeOut*1000);
        }
        printf("DISC received!\n");
        int j = 0;
        for (; j < connectionParameters.numTries; j++)
        {
            res = send_message(fd, DISC, buff, BCC1_FIELD_LENGTH, role);
            printf("DISC sent! num of try : %d\n", j);
            usleep(connectionParameters.timeOut*1000);
            if(check_for_UA(fd, role)) break;
        }
    }

    return 1;
}

int check_for_DISC(int fd, int role)
{
    int res = 0;
    char c_read = ' ';
    int n_pass = TRUE;
    int ret = 0;
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char BCC1 = addr ^ DISC;
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
                if (c_read == addr) {n_pass = FALSE;}
                else if (c_read == FLAG) continue;
                else state = 0;
                break;
            case 2:
                //printf("read 2 - %x \n", c_read);
                if (c_read == DISC) {n_pass = FALSE; ret = 1;}
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