#include<fragment.h>

struct ack_waiting
{
    frag_id_t fid;
	packet_id_t pid;
    int send_time;
};

struct ack_waiting * ack_waiting_create(packet_id_t pid, frag_id_t fid, int send_time);

void ack_waiting_destroy(struct ack_waiting * aw);

int8_t ack_waiting_cmp(const void * aw1, const void *aw2);