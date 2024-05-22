#include "../include/shared.h"

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

int append_to_packet(struct packet* packet, char* buff, size_t buff_length)
{
    int new_size = packet->size + buff_length;

    char* hbuffer = realloc(packet->buffer, sizeof(char) * new_size);
    if(hbuffer == NULL) return 0; //realloc error

    packet->buffer = hbuffer;

    // skip packet->size bytes and append from buffer, buff_length bytes to packet->buffer
    memcpy(packet->buffer + packet->size, buff, buff_length);

    packet->first = packet->buffer;
    packet->size = new_size;
    packet->last = packet->buffer + new_size - 1;

    return 1;
}

bool clean_packet(struct packet* packet)
{
    if(packet->buffer != NULL)
    {
        free(packet->buffer);
        packet->buffer = NULL;
    }
    packet->buffer = NULL;
    packet->first = NULL;
    packet->last = NULL;
    packet->size = 0;
}

bool construct_BCC1(enum SIGNALS type, int role, char* buff, int length)
{
    char addr = role ? RECV_ADDR : TRANS_ADDR;
    char cntrl = type;
    char BCC1 = addr ^ cntrl;

    if(length >= BCC1_FIELD_LENGTH)
    {
        buff[0] = FLAG;
        buff[1] = addr;
        buff[2] = cntrl;
        buff[3] = BCC1;

    } else { perror("construct_BCC1"); return ERROR; }

    return TRUE;
}

//this stalls when bcc1 is wrong, probably not intended!!
int validate_packet_BCC1(int fd, struct packet* packet, enum SIGNALS* type, int role, int retr_count)
{
    int res = 1;
    char c_read = ' ';
    char buffer[BCC1_FIELD_LENGTH];
    char BCC1 = ' ';
    int state = 0;

    /*while(res > 0)
    {
        res = read(fd, &c_read, 1);
        printf("%x", c_read);
    }
    printf("\n");*/

    int p = 0;
    while (state < BCC1_FIELD_LENGTH) {
        res = read_buffer(fd, &c_read, 1);
        if (res <= 0) return 0;

        //if (!p) { printf("Header : \n"); p = 1;}
        switch (state) {
            case 0:
                printf(" 0 - %x \n", c_read);
                if (c_read == FLAG)
                {
                    buffer[state] = FLAG;
                    state++;
                } else return 0;
                break;
            case 1:
                printf(" 1 - %x \n", c_read);
                if (c_read == TRANS_ADDR)
                {
                    buffer[state] = TRANS_ADDR;
                    state++;
                } else return 0;
                break;
            case 2:
                printf(" 2 - %x \n", c_read);
                if (c_read == I1 || c_read == (char)'\x80') // da
                {
                    buffer[state] = c_read;
                    BCC1 = TRANS_ADDR ^ c_read;
                    *type = I1;
                    state++;
                } else return 0;
                break;
            case 3:
                printf(" 3 - %x \n", c_read);
                if (c_read == BCC1)
                {
                    buffer[state] = BCC1;
                    state++;
                } else return 0;
                break;
            default:
                return 0;
        }
    }
    printf("Header end: \n");

    if (state == BCC1_FIELD_LENGTH)
    {
        append_to_packet(packet, buffer, BCC1_FIELD_LENGTH);
        return 1;
    }

    return 1;
}

char calculate_BCC2(char* data, int data_length)
{
    char result = data[0];
    for (int i = 1; i < data_length; i++)
    {
        result = result ^ data[i];
    }
    return result;
}