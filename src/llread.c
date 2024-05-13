#include "../include/llread.h"

int llread(char* buffer)
{
    int fd;

    char* s_port = "/dev/ttyS11";
    fd = open(s_port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(s_port);
        exit(-1);
    }

    struct packet packet;
    get_packet(fd, packet);

}


enum SIGNALS geT_packet(int fd, struct packet packet)
{

    enum SIGNALS type = ERROR;
    int retr_count_1 = 0;
    int retr_count_2 = 0;

    if (validate_packet_BCC1(fd, packet, &type, retr_count_1)) return ERROR;
    if (validate_packet_BCC2(fd, packet, &type, retr_count_2)) return ERROR;

    return type;
}

int validate_packet_BCC1(int fd, struct packet packet, enum SIGNALS* type, int retr_count)
{
    int res = 0, n_pass = TRUE, exit = 0;
    char c_read = ' ';
    char BCC1 = ' ';
    for (int state = 0; exit < 1; state++)
    {
        printf("state - %d\n", state);
        printf("pass - %d\n", n_pass);
        n_pass = TRUE;
        while(res >= 0 && n_pass)   // infinitte loops
        {
            res = read_buffer(fd, &c_read, 1);
            if (!res) continue;                  // fix

            switch (state)
            {
            case 0:
                printf("read 0 - %x\n", c_read);
                if (c_read == FLAG) {n_pass = FALSE;}
                break;
            case 1:
                printf("read 1 - %x \n", c_read);
                if (c_read == TRANS_ADDR) {n_pass = FALSE;}
                else if (c_read == FLAG) continue;
                else state = 0;
                break;
            case 2:
                printf("read 2 - %x \n", c_read);
                if (c_read == I0) {n_pass = FALSE; BCC1 = TRANS_ADDR ^ I0;}
                else if (c_read == I1) {n_pass = FALSE; BCC1 = TRANS_ADDR ^ I1;}
                else if (c_read == FLAG) state = 1;
                else state = 0;
                break;
            case 3:
                printf("read 3 - %x \n", c_read);
                if (c_read == BCC1) {n_pass = FALSE; exit = 1; *type = BCC1 ^ TRANS_ADDR;}
                else if (c_read == FLAG) {state = 1;}
                else state = 0;
                break;
            default:
                break;
            }
        }
    }

    char buffer[4];
    buffer[0] = FLAG;
    buffer[1] = TRANS_ADDR;
    buffer[2] = type;
    buffer[3] = BCC1;
    append_to_packet(&packet, buffer, 4);

    return exit;
}

int validate_packet_BCC2(int fd, struct packet packet, enum SIGNALS* type, int retr_count)
{
    int res = 0, n_pass = TRUE, exit = 0, ret = 0;;
    char c_read = ' ';
    char BCC2 = ' ';
    char buffer[256];
    for (int state = 0; exit < 1; state++)
    {
        printf("state - %d\n", state);
        printf("pass - %d\n", n_pass);
        n_pass = TRUE;  
        while(res >= 0 && n_pass)   // infinitte loops
        {
            res = read_buffer(fd, &c_read, 1);
            if (!res) continue;                  // fix

            switch (state)
            {
            case 0:                             
                // data case
                printf("read 0 - %x\n", c_read);
                //when leaving add a '\n' or some shit to indicate the end of the data on buffer
                if (c_read == BCC2) {n_pass = FALSE;}               //waht if data is flag
                break;
            case 1:
                printf("read 1 - %x \n", c_read);
                if (c_read == FLAG) {n_pass = FALSE; ret = 1;}
                else state = 0;
                break;
            default:
                break;
            }
        }
    }

    return 1;
}

int read_buffer(int fd, char* buff, int length)
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

int append_to_packet(struct packet* packet, char* buffer, size_t buff_length)
{
    int new_size = packet->size + buff_length;

    packet->buffer = realloc(packet->buffer, sizeof(char) * new_size);
    if(packet->buffer == NULL) return 0;

    for(int i = 0; i < buff_length; i++)
    {
        packet->buffer[i + packet->size] = buffer[i];
    }

    if(packet->size == 0) packet->first = packet->buffer[0];
    packet->size = new_size;
    packet->last = packet->buffer[new_size - 1];

    return 1;
}


