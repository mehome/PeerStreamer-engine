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

#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__

#include<net_msg.h>

typedef uint16_t frag_id_t;
typedef uint16_t packet_id_t;

//typedef enum fragment_type {FRAGMENT_TYPE_NORMAL, FRAGMENT_TYPE_RELIABLE} Fragment_type;
#define FRAGMENT_TYPE_NORMAL 0
#define FRAGMENT_TYPE_RELIABLE 1
typedef uint8_t frag_type;

struct fragment {  // extends net_msg, do not move nm parameter
	struct net_msg nm;
	frag_id_t id;
	frag_id_t frag_num;
	packet_id_t pid;
	frag_type type;
	size_t data_size;
	uint8_t * data;
};

//int8_t fragment_init(struct fragment * f, const struct nodeID * from, const struct nodeID * to, packet_id_t pid, frag_id_t frag_num, frag_id_t id, const uint8_t * data, size_t data_size, struct list_head * list);

int8_t fragment_init(struct fragment * f, const struct nodeID * from, const struct nodeID * to, packet_id_t pid, frag_id_t frag_num, frag_id_t id, frag_type type, const uint8_t * data, size_t data_size, struct list_head * list);

void fragment_deinit(struct fragment * f);

struct list_head * fragment_list_element(struct fragment *f);

ssize_t fragment_send(int sockfd, const struct sockaddr *dest_addr, socklen_t addrlen, struct fragment * f, uint8_t * buff, size_t buff_len);

struct fragment * fragment_decode(const struct nodeID *dst, const struct nodeID *src, const uint8_t * buff, size_t buff_len);

int8_t fragment_encode(struct fragment * frag, uint8_t * buff, size_t buff_len);

#endif
