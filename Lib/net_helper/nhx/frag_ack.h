#ifndef __FRAG_ACK_H__
#define __FRAG_ACK_H__

#include<net_msg.h>
#include<fragment.h>

struct frag_ack {  // extends net_msg, do not move nm parameter
	struct net_msg nm;
	frag_id_t id;
	packet_id_t pid;
};

struct frag_ack * frag_ack_create(const struct nodeID * from, const struct nodeID * to, packet_id_t pid, frag_id_t fid, struct list_head * list);

void frag_ack_destroy(struct frag_ack ** fr);

struct list_head * frag_ack_list_element(struct frag_ack *f);

ssize_t frag_ack_send(int sockfd, const struct sockaddr *dest_addr, socklen_t addrlen, struct frag_ack * fr, uint8_t * buff, size_t buff_len);

struct frag_ack * frag_ack_decode(const struct nodeID *dst, const struct nodeID *src, const uint8_t * buff, size_t buff_len);

int8_t frag_ack_encode(struct frag_ack *fr, uint8_t * buff, size_t buff_len);

#endif
