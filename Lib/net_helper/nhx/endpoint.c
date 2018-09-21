/*
 * Copyright (c) 2017 Luca Baldesi
 *
 * This file is part of PeerStreamer.
 *
 * PeerStreamer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * PeerStreamer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with PeerStreamer.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include<endpoint.h>
#include<net_helper.h>
#include<packet_bucket.h>
#include<fragment.h>
#include<frag_ack.h>
#include<ack_waiting.h>
#include<ord_set.h>


#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define WAITING_ACKS_DEFAULT_SIZE 10
#define ACK_WAITING_TIME 10

struct endpoint {  // do not move node parameter
	struct nodeID * node;
	struct packet_bucket * incoming;
	struct packet_bucket * outgoing;
	packet_id_t out_id;
	struct ord_set * waiting_acks;
};

struct list_head * endpoint_enqueue_outgoing_packet(struct endpoint * e, const struct nodeID * src, const uint8_t * data, size_t data_len)
{
	struct list_head * res = NULL;
	if (e && src && data && data_len > 0)
		res = packet_bucket_add_packet(e->outgoing, src, e->node, e->out_id++, data, data_len);
	return res;
}

struct endpoint * endpoint_create(const struct nodeID * node, size_t frag_size, uint16_t max_pkt_age)
{
	struct endpoint * e = NULL;
	if (node)
	{
		e = malloc(sizeof(struct endpoint));
		e->node = nodeid_dup(node);
		e->incoming = packet_bucket_create(frag_size, max_pkt_age);
		e->outgoing = packet_bucket_create(frag_size, max_pkt_age);
		e->waiting_acks = ord_set_new(WAITING_ACKS_DEFAULT_SIZE, ack_waiting_cmp);
		e->out_id = 0;
	}
	return e;
}

void endpoint_destroy(struct endpoint ** e)
{
	if (e && *e)
	{
		packet_bucket_destroy(&(*e)->incoming);
		packet_bucket_destroy(&(*e)->outgoing);
		nodeid_free((*e)->node);
		ord_set_destroy(&((*e)->waiting_acks), 1);
		free(*e);
		*e = NULL;
	}
}

int8_t endpoint_cmp(const void * e1, const void *e2)
{
	if (e1 && e2)
		if (nodeid_equal(((struct endpoint *)e1)->node, ((struct endpoint *)e2)->node))
			return 0;
		return nodeid_cmp(((struct endpoint *)e1)->node, ((struct endpoint *)e2)->node) > 0 ? 1 : -1;
	return 0;
}

struct nodeID * endpoint_get_node(struct endpoint * e)
{
	return e->node;
}

packet_state_t endpoint_add_incoming_fragment(struct endpoint * e, const struct fragment *f, struct list_head * requests)
{
	return packet_bucket_add_fragment(e->incoming, f, requests);
}

int8_t endpoint_pop_incoming_packet(struct endpoint *e, packet_id_t pid, uint8_t * buff, size_t * size)
{
	return packet_bucket_pop_packet(e->incoming, pid, buff, size);
}

struct fragment * endpoint_get_outgoing_fragment(struct endpoint *e, packet_id_t pid, frag_id_t fid)
{
	return packet_bucket_get_fragment(e->outgoing, pid, fid);
}

void endpoint_push_waiting_ack(struct endpoint *e, packet_id_t pid, frag_id_t fid, struct timeval send_time)
{
	fprintf(stderr, "endpoint_push_waiting_ack \n");
	struct ack_waiting *newAW;
	newAW = (struct ack_waiting *)ack_waiting_create(pid, fid, send_time);
	ord_set_insert(e->waiting_acks, (void *)newAW, 1);
}

int8_t endpoint_remove_waiting_ack(struct endpoint *e, packet_id_t pid, frag_id_t fid)
{
	fprintf(stderr, "endpoint_remove_waiting_ack \n");
	int8_t res = -1;
	struct ack_waiting *i = NULL;
	do
	{
		i = (struct ack_waiting *)ord_set_iter(e->waiting_acks, (void *)i);
		if(i && i->pid == pid && i->fid == fid)
		{			
			res = ord_set_remove(e->waiting_acks, i, 1);
			i = NULL;
			res = 0;
		}
	}while(i);

	return res;
}

struct ack_waiting * endpoint_pop_waiting_ack(struct endpoint *e, struct timeval now)
{
	fprintf(stderr, "endpoint_pop_waiting_ack \n");
	struct ack_waiting *fAW;
	fAW = ord_set_iter(e->waiting_acks, NULL);

	struct timeval waitTime;
	waitTime.tv_sec = ACK_WAITING_TIME;
	struct timeval expireTime;
	timeradd(&(fAW->send_time), &waitTime, &expireTime);
	if(timercmp(&expireTime, &now, >))
		fAW = NULL;
	else	
		ord_set_remove(e->waiting_acks, fAW, 0);

	return fAW;
}

int8_t endpoint_send_packet_reliable(struct endpoint * e, const struct nodeID * src, size_t frag_size, const uint8_t * data, size_t data_len)
{
	int8_t res = -1;

	int frag_num;
	frag_type type = FRAGMENT_TYPE_RELIABLE;

	if(e && src && data && data_len > 0)
	{
		if (!frag_size)
			frag_size = data_len;
		frag_num = data_len/frag_size;
		if (data_len % frag_size)
			frag_num++;

		e->out_id++;
		
		for(int i = 0; i < frag_num; i++)
		{
			struct fragment *newFragment = malloc(sizeof(struct fragment));
			int8_t res = fragment_init(newFragment, src, e->node, e->out_id, frag_num, i, type, data+(i*frag_size), MIN(frag_size, data_len-(i*frag_size)), NULL);
			
			if(res == 0){
				net_helper_send_msg(src, (struct net_msg *)newFragment );
				struct timeval now_time;
				gettimeofday(&now_time, NULL);
				endpoint_push_waiting_ack(e, e->out_id, i, now_time);
			}
				

			fragment_deinit(newFragment);
			free(newFragment);
		}
		res = data_len;
	}

	return res;
}

int8_t endpoint_send_ack(struct endpoint *e, const struct nodeID *src, packet_id_t pid, frag_id_t id)
{
	int8_t res = -1;

	if(e && src)
	{
		struct frag_ack *newAck;
		newAck = frag_ack_create(src, e->node, pid, id, NULL);
		if(newAck){
			net_helper_send_msg(src, (struct net_msg *) newAck);
			res = 0;

			frag_ack_destroy(&newAck);
		}		
	}

	return res;
}

int8_t endpoint_receive_ack(struct endpoint *e, packet_id_t pid, frag_id_t fid)
{
	return endpoint_remove_waiting_ack(e, pid, fid);
}
