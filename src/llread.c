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

    struct packet packet = 
    {
        .buffer = NULL,
        .first = NULL,
        .last = NULL,
        .size = 0,
    };
        
    enum SIGNALS ret_type;
    int read = 0;
    while(1)
    {
        if(ret_type = get_packet(fd, &packet, &read))
        {
            printf("size received - %d\n", packet.size);
            send_response(fd, ret_type);
            clean_packet(&packet);
            packet_to_buffer(&packet, buffer);

            return read;
        }
    }

    return -1;
}


enum SIGNALS get_packet(int fd, struct packet* packet, int* read)
{

    enum SIGNALS type = ERROR;
    int retr_count_1 = 0;
    int retr_count_2 = 0;

    *read = 0;

    int c = 0;
    if (!validate_packet_BCC1(fd, packet, &type, 0, retr_count_1)) return ERROR;
    else *read = 5;
    if ( (c = validate_packet_BCC2(fd, packet, &type, retr_count_2)) <= 0)
    {
        send_response(fd, REJ);
        //how do i know if its duplicate??
        return ERROR;
    }else *read = 5 + c;

    return type;    
}

int validate_packet_BCC2(int fd, struct packet* packet, enum SIGNALS* type, int retr_count)
{
    int res = 1, n_pass = TRUE, ret = 0, count = 0;
    char c_read = ' ';
    char BCC2 = ' ';               
    char buffer[2048];
    

    printf("Data: \n");
    while(res >= 0 && n_pass)   // infinitte loops
    {
        res = read_buffer(fd, &c_read, 1);
        if(!res) continue;

        printf("%x ", c_read);
        if (c_read == FLAG)
        {
            n_pass = FALSE;
            printf("\n");
            char actual_bcc2 = calculate_BCC2(buffer, count);
            char expected_bcc2 = buffer[count - 1];
            printf("BCC2 : %x -- %x \n", actual_bcc2, expected_bcc2);
            if(actual_bcc2 == expected_bcc2) buffer[count] = FLAG;
            else return 0;
            break;
        }
        else if(c_read == ESCAPE)
        {
            printf("escape - %x\n", c_read);
            res = read_buffer(fd, &c_read, 1);
            if(res != 0)
            {
                if (c_read == SIGNAL_FLAG) c_read = FLAG;
                else if (c_read == SIGNAL_ESCAPE) c_read = ESCAPE;
                else
                // case where we just read a lonely escape, c_read is whatever is after escape and will be added below
                {
                    buffer[count] = ESCAPE;
                    count++;
                }
            }
        }

        buffer[count] = c_read;
        count++;
        //printf("c %d \n", count);
    }
    printf("Data end: \n");
    
    append_to_packet(packet, buffer, count + 2);

    return count + 2;
}


bool send_response(int fd, enum SIGNALS type)
{
    enum SIGNALS t;
    if (type == I0 || type == I1) t = RR1;
    else t = RR0;

    char buff[BCC1_FIELD_LENGTH];
    construct_BCC1(t, 1, buff, BCC1_FIELD_LENGTH);

    int res = write(fd, buff, BCC1_FIELD_LENGTH);
    if(res <= 0) return ERROR;
    
    return TRUE;
}

void packet_to_buffer(struct packet* packet, char* buffer)
{
    for (int i = 0; i < packet->size; i++)
    {
        buffer[i] = packet->buffer[i];
    }
}


