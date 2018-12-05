#include<ack_waiting.h>
#include<stdio.h>

struct ack_waiting * ack_waiting_create(struct fragment *frag, struct timeval send_time, uint8_t n_resend)
{
    struct ack_waiting * n;
    n = malloc(sizeof(struct ack_waiting));

    n->frag = fragment_dup(frag);
    n->send_time = send_time;
    n->n_resend = n_resend;

    return n;
}

void ack_waiting_destroy(struct ack_waiting * aw)
{
    fragment_deinit(aw->frag);
    free(aw->frag);
    free(aw);
}

int8_t ack_waiting_cmp(const void * pt1, const void *pt2)
{
    int8_t res = 0;
    if(pt1 && pt2)
    {
        struct ack_waiting * aw1 = (struct ack_waiting *)pt1;
        struct ack_waiting * aw2 = (struct ack_waiting *)pt2;

        if(timercmp(&(aw1->send_time), &(aw2->send_time), ==)) 
        {
            if(aw1->frag->pid == aw2->frag->pid)
            {
                if(aw1->frag->id == aw2->frag->id)
                {
                    res = 0;
                }
                else
                {
                    res = aw1->frag->id > aw2->frag->id ? 1 : -1; 
                }
            }
            else
            {
                res = aw1->frag->pid > aw2->frag->pid ? 1 : -1; 
            }
        }
        else
        {
            res = timercmp(&(aw1->send_time), &(aw2->send_time), >) ? 1 : -1; 
        }
        
    }
    return res;
}