#include<fragment.h>
#include<sys/time.h>

struct ack_waiting
{
    struct fragment *frag;
    struct timeval send_time;
};

struct ack_waiting * ack_waiting_create(struct fragment *frag, struct timeval send_time);

void ack_waiting_destroy(struct ack_waiting * aw);

int8_t ack_waiting_cmp(const void * aw1, const void *aw2);