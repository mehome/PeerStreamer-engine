#include<ack_waiting.h>

struct ack_waiting * ack_waiting_create(packet_id_t pid, frag_id_t fid, int send_time)
{
    struct ack_waiting * n;
    n = malloc(sizeof(struct ack_waiting));

    n->fid = fid;
    n->pid = pid;
    n->send_time = send_time;

    return n;
}

void ack_waiting_destroy(struct ack_waiting * aw)
{
    free(aw);
}

int8_t ack_waiting_cmp(const void * pt1, const void *pt2)
{
    int8_t res = 0;
    if(pt1 && pt2)
    {
        struct ack_waiting * aw1 = (struct ack_waiting *)pt1;
        struct ack_waiting * aw2 = (struct ack_waiting *)pt2;

        if(aw1->send_time == aw2->send_time) 
        {
            if(aw1->pid == aw2->pid)
            {
                if(aw1->fid == aw2->fid)
                {
                    res = 0;
                }
                else
                {
                    res = aw1->fid > aw2->fid ? 1 : -1; 
                }
            }
            else
            {
                res = aw1->pid > aw2->pid ? 1 : -1; 
            }
        }
        else
        {
            res = aw1->send_time > aw2->send_time ? 1 : -1; 
        }
        
    }
    return res;
}