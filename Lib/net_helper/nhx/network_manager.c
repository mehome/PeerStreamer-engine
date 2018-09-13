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

#include<network_manager.h>
#include<malloc.h>
#include<endpoint.h>
#include<ord_set.h>
#include<grapes_config.h>
#include<frag_request.h>
#include<frag_ack.h>

#define DEFAULT_PKT_MAX_AGE 4


struct network_manager {
	struct list_head outqueue;
	struct ord_set * endpoints;
	size_t frag_size;
	uint16_t max_pkt_age; // in seconds
};

struct network_manager * network_manager_create(const char * config)
{
	struct network_manager * nm ;
	struct tag * tags = NULL;
	int frag_size = DEFAULT_FRAG_SIZE;
	int max_pkt_age = DEFAULT_PKT_MAX_AGE;


	nm = malloc(sizeof(struct network_manager));
	nm->endpoints = ord_set_new(10, endpoint_cmp);
	INIT_LIST_HEAD(&(nm->outqueue));

	if (config)
	{
		tags = grapes_config_parse(config);
		grapes_config_value_int_default(tags, "frag_size", &frag_size, DEFAULT_FRAG_SIZE);
		grapes_config_value_int_default(tags, "max_pkt_age", &max_pkt_age, DEFAULT_PKT_MAX_AGE);
		free(tags);
	}
	nm->frag_size = frag_size;
	nm->max_pkt_age = max_pkt_age;
	return nm;
}

void network_manager_destroy(struct network_manager ** nm)
{
	void * e, * tmp;
	struct list_head *pos, *next;
	struct frag_request * fr;

	if (nm && *nm)
	{
		ord_set_for_each_safe(e, (*nm)->endpoints, tmp)
		{
			ord_set_remove((*nm)->endpoints, (void *) e, 0);
			endpoint_destroy((struct endpoint **)&e);	
		}

		list_for_each_safe(pos, next, &((*nm)->outqueue))
		{
				fr = (struct frag_request *) list_entry(pos, struct net_msg, list);
				frag_request_destroy(&fr);
		}

		ord_set_destroy(&((*nm)->endpoints), 0);
		free(*nm);
		*nm = NULL;
	}
}

void network_manager_print_outqueue(const struct network_manager *nm)
{
	struct list_head * pos;
	struct fragment * msg;
	uint16_t i=0;

	fprintf(stderr, "=== Outqueue ===\n");
	list_for_each(pos, &(nm->outqueue))
	{	
		msg = (struct fragment *) list_entry(pos, struct net_msg, list);
		fprintf(stderr, "%d) frag_id = %d\n", i++, msg->id); 
	}
} 

int8_t network_manager_enqueue_outgoing_packet(struct network_manager *nm, const struct nodeID *src, const struct nodeID * dst, const uint8_t * data, size_t data_len)
{
	int8_t res = -1;
	struct endpoint * e;
	struct list_head * frag_list;

	if (nm && dst && data && data_len > 0)
	{
		e = ord_set_find(nm->endpoints, &dst);
		if (!e)
		{
			e = endpoint_create(dst, nm->frag_size, nm->max_pkt_age);
			ord_set_insert(nm->endpoints, (void *)e, 0);
		}
		frag_list = endpoint_enqueue_outgoing_packet(e, src, data, data_len);
		if (frag_list)
		{
			list_splice(frag_list, &(nm->outqueue));
			free(frag_list);
			res = 0;
		}
	}

	return res;
}

int8_t network_manager_enqueue_outgoing_packet_reliable(struct network_manager *nm, const struct nodeID *src, const struct nodeID * dst, const uint8_t * data, size_t data_len)
{
	int8_t res = -1;
	struct endpoint * e;
	struct list_head * frag_list;

	if (nm && dst && data && data_len > 0)
	{
		e = ord_set_find(nm->endpoints, &dst);
		if (!e)
		{
			e = endpoint_create(dst, nm->frag_size, nm->max_pkt_age);
			ord_set_insert(nm->endpoints, (void *)e, 0);
		}
		frag_list = endpoint_enqueue_outgoing_packet_reliable(e, src, data, data_len);
		if (frag_list)
		{
			list_splice(frag_list, &(nm->outqueue));
			free(frag_list);
			res = 0;
		}
	}

	return res;
}

struct net_msg * network_manager_pop_outgoing_net_msg(struct network_manager *nm)
{
	struct net_msg * m = NULL;
	struct list_head * el;

	if (nm)
	{
		el = list_pop(&(nm->outqueue));
		if (el)
		{
			m = list_entry(el, struct net_msg, list);
		}			
	}
	return m;
}

packet_state_t network_manager_add_incoming_fragment(struct network_manager * nm, const struct fragment * f)
{
	packet_state_t res = PKT_ERROR;
	struct list_head requests;
	struct endpoint * e;
	const struct nodeID * from;

	if (nm && f)
	{
		from = ((struct net_msg *)f)->from;
		e = ord_set_find(nm->endpoints, &from);
		if (!e)
		{
			e = endpoint_create(from, nm->frag_size, nm->max_pkt_age);
			ord_set_insert(nm->endpoints, (void *)e, 0);
		}
		INIT_LIST_HEAD(&requests);
		res = endpoint_add_incoming_fragment(e, f, &requests);
		list_splice(&requests, &(nm->outqueue));
	}
	return res;
}

int8_t network_manager_pop_incoming_packet(struct network_manager *nm, const struct nodeID * src, packet_id_t id, uint8_t * buff, size_t *size)
{
	int8_t res = -1;
	struct endpoint * e;

	if (nm && src && size && buff)
	{
		e = ord_set_find(nm->endpoints, &src);
		if (e)
			res = endpoint_pop_incoming_packet(e, id, buff, size);
		else
			res = -2;

	}
	return res;
}

int8_t network_manager_enqueue_outgoing_fragment(struct network_manager *nm, const struct nodeID * dst, packet_id_t id, frag_id_t fid)
{
	int8_t res = -1;  // invalid input
	struct endpoint * e;
	struct fragment * f = NULL;

	if (nm && dst)
	{
		res = -2;  // endpoint/packet/fragment not found
		e = ord_set_find(nm->endpoints, &dst);
		if (e)
			f = endpoint_get_outgoing_fragment(e, id, fid);
		if (f)
		{
			if (list_element_notadded(fragment_list_element(f)))
			{
				res = 0;  // ok
				list_add_tail(fragment_list_element(f), &(nm->outqueue));
			} else
				res = 1;  // fragment already in sending queue
		}
	}

	return res;
}

int8_t network_manager_outgoing_queue_ready(struct network_manager *nm)
{
	if (nm && !list_empty(&(nm->outqueue)))
		return 1;
	return 0;
}
