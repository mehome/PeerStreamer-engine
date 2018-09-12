#include<fragment.h>
#include<frag_ack.h>
#include<string.h>
#include<int_coding.h>

#define FRAG_ACK_HEADER_LEN (sizeof(net_msg_t) + sizeof(packet_id_t) + sizeof(frag_id_t))

struct frag_ack * frag_ack_create(const struct nodeID * from, const struct nodeID * to, packet_id_t pid, frag_id_t fid, struct list_head * list)
{
    struct frag_ack * fr;

	fr = malloc(sizeof(struct frag_ack));
	net_msg_init((struct net_msg *) fr, NET_FRAGMENT_ACK, from, to, list);
	fr->pid = pid;
	fr->id = fid;

	return fr;
}

void frag_ack_destroy(struct frag_ack ** fr)
{
    if (fr && *fr)
	{
		net_msg_deinit((struct net_msg *)*fr);
		free(*fr);
		*fr = NULL;
	}
}

struct list_head * frag_ack_list_element(struct frag_ack *f)
{
    if (f)
		return &((struct net_msg*)f)->list;
	return NULL;
}

int8_t frag_ack_encode(struct frag_ack *fr, uint8_t * buff, size_t buff_len)
{
    int8_t res = -1;
	uint8_t * ptr;

	ptr = buff;
	if (fr && buff && buff_len >= FRAG_ACK_HEADER_LEN)
	{
		*((net_msg_t*) ptr) = NET_FRAGMENT_ACK;
		ptr += 1;
		int16_cpy(ptr, fr->pid);
		ptr += 2;
		int16_cpy(ptr, fr->id);
		ptr += 2;
		
		res = 0;
	}
	return res;
}

ssize_t frag_ack_send(int sockfd, const struct sockaddr *dest_addr, socklen_t addrlen, struct frag_ack * fr, uint8_t * buff, size_t buff_len)
{
    ssize_t res = -1;

	if (dest_addr && fr && buff && buff_len >= FRAG_ACK_HEADER_LEN)
	{
		frag_ack_encode(fr, buff, buff_len);
		
		res = sendto(sockfd, buff, FRAG_ACK_HEADER_LEN, MSG_CONFIRM, dest_addr, addrlen);
	}
	return res;
}

struct frag_ack * frag_ack_decode(const struct nodeID *dst, const struct nodeID *src, const uint8_t * buff, size_t buff_len)
{
    struct frag_ack * msg;
	const uint8_t * ptr;
	packet_id_t pid;
	frag_id_t fid;

	if (dst && src && buff && buff_len >= FRAG_ACK_HEADER_LEN)
	{
		ptr = buff + 1;
		pid = int16_rcpy(ptr);
		ptr = ptr + 2;
		fid = int16_rcpy(ptr);
		msg = frag_ack_create(src, dst, pid, fid, NULL);
	}

	return msg;
}

