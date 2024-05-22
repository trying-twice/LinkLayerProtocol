#include "../include/llwrite.h"

int llwrite(char* buffer, int length)
{
    int fd;

    char* s_port = "/dev/ttyS10";
    fd = open(s_port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(s_port);
        exit(-1);
    }

    char buff1[length + BCC1_FIELD_LENGTH + 2];

    struct packet packet = 
    {
        .buffer = NULL,
        .first = NULL,
        .last = NULL,
        .size = 0,
    };
    char type = I0;
    char buff[100] = { 0 };

    int retr_count = 0;
    int res = 0;
    while(1)
    {
        res = send_packet(fd, &packet, type, buff, 100);

        enum SIGNALS t = ERROR;
        bool clean = FALSE;
        clock_t start_time = clock();
        clock_t elapsed_time;
        double elapsed_seconds = 0;
        for (int i = 0; ; i++)
        {
            t = read_response(fd);

            elapsed_time = clock() - start_time;
            elapsed_seconds = ((double)elapsed_time) / CLOCKS_PER_SEC;
            if(elapsed_seconds >= 4.0) break;

            switch (t)
            {
                case REJ:
                    printf("Received REJ!\n");
                    retr_count++;
                    break;
                case RR1:
                    printf("Received RR1!\n");
                    clean = TRUE;
                    retr_count = 0;
                    break;
                case RR0:
                    printf("Received RR0!\n");
                    clean = TRUE;
                    retr_count = 0;
                    break;
                default:
                    break;
            }
            if(t == (unsigned char)"\x11")
            {
                printf("Received R11R1!\n");
                clean = TRUE;
                retr_count = 0;
                break; 
            }

        }

        if(clean) clean_packet(&packet);

    }

    return res;
}

int send_packet(int fd, struct packet* packet, char type, char* data, int data_length)
{
    int retr_count_1 = 0;
    int retr_count_2 = 0;

    if(!construct_packet(packet, type, data, data_length)) return ERROR;

    printf("size sent - %d\n", packet->size);

    int res;

    for(int i = 0; i < packet->size; i++)
    {
        if(i < 5) printf("%x", packet->buffer[i]);
        else printf("%x", packet->buffer[i]);
    }
    printf("\n");
    res = write(fd, packet->buffer, packet->size);
    if(!res) return ERROR;

    printf("size sent - %d, n written - %d\n", packet->size, res);
    return res;
}
bool construct_packet(struct packet* packet, char type, char* data, int data_length)
{
    int l = BCC1_FIELD_LENGTH + data_length + 2;
    char buff[l];
    memset(buff, 0, l);

    if(!construct_BCC1(type, 0, buff, BCC1_FIELD_LENGTH)) return ERROR;
    if(!construct_BCC2(data, data_length, buff)) return ERROR;

    if(!append_to_packet(packet, buff, l)) return ERROR;

    return TRUE;
}

bool construct_BCC2(char* data, int data_length, char* dest)
{
    if (data_length <= 0) return FALSE;
    memcpy(dest + BCC1_FIELD_LENGTH, data, data_length);
    dest[BCC1_FIELD_LENGTH + data_length] = calculate_BCC2(data, data_length);
    printf("BCC2 - %x\n", calculate_BCC2(data, data_length));
    dest[BCC1_FIELD_LENGTH + data_length + 1] = FLAG;

    return TRUE;
}

enum SIGNALS read_response(int fd)
{

    char buff[BCC1_FIELD_LENGTH];

    int res = read_buffer(fd, buff, BCC1_FIELD_LENGTH);
    if(res <= 0) return ERROR;

    printf("read response - %d --- %x \n", res, buff[2]);

    return buff[2];
}
