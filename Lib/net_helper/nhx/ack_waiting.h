#include<fragment.h>
#include<sys/time.h>

struct ack_waiting
{
    struct fragment *frag;
    struct timeval send_time;
    uint8_t n_resend;
};

struct ack_waiting * ack_waiting_create(struct fragment *frag, struct timeval send_time, uint8_t n_resend);

void ack_waiting_destroy(struct ack_waiting * aw);

int8_t ack_waiting_cmp(const void * aw1, const void *aw2);