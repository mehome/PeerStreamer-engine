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

#include<fragmented_packet.h>
#include<frag_request.h>
#include<string.h>
#include<sys/time.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif 

time_t fragmented_packet_creation_timestamp(const struct fragmented_packet *fp)
{
	if (fp)
		return fp->creation_timestamp;
	return 0;
}

packet_id_t fragmented_packet_id(const struct fragmented_packet *fp)
{
	if (fp)
		return fp->packet_id;
	return 0;
}

struct fragmented_packet * fragmented_packet_create(packet_id_t id, const struct nodeID * from, const struct nodeID *to, const uint8_t * data, size_t data_size, size_t frag_size, frag_type type, struct list_head * msgs)
{
	struct fragmented_packet * fp = NULL;
	const uint8_t * data_ptr = data;
	frag_id_t i;

	if (data && data_size > 0 && frag_size > 0)
	{
		fp = malloc(sizeof(struct fragmented_packet));
		fp->packet_id = id;
		fp->creation_timestamp = time(NULL);
		INIT_LIST_HEAD(&(fp->list));
		if (!frag_size)
			frag_size = data_size;
		fp->frag_num = data_size/frag_size;
		if (data_size % frag_size)
			fp->frag_num++;
		fp->frags = malloc(sizeof(struct fragment) * fp->frag_num);
		fp->type = type;
		for (i = 0; i < fp->frag_num; i++)
		{
			fragment_init(&(fp->frags[i]), from, to, id, fp->frag_num, i, type, data+(i*frag_size), MIN(frag_size, data_size), msgs);
			data_ptr += frag_size; 
			data_size -= frag_size;
		}
	}
	return fp;
}

void fragmented_packet_destroy(struct fragmented_packet ** fp)
{
	frag_id_t i;

	if (fp && *fp)
	{
		for (i = 0; i < (*fp)->frag_num; i++)
			fragment_deinit(&(*fp)->frags[i]);
		free((*fp)->frags);
		free(*fp);
		*fp = NULL;
	}
}

struct fragmented_packet * fragmented_packet_empty(packet_id_t pid, const struct nodeID *from, const struct nodeID *to, frag_id_t num_frags, frag_type type)
{
	struct fragmented_packet * fp = NULL;
	frag_id_t i;

	fp = malloc(sizeof(struct fragmented_packet));
	fp->packet_id = pid;
	fp->creation_timestamp = time(NULL);
	INIT_LIST_HEAD(&(fp->list));
	fp->frag_num = num_frags;
	fp->frags = malloc(sizeof(struct fragment) * fp->frag_num);
	for (i = 0; i < fp->frag_num; i++)
		fragment_init(&(fp->frags[i]), from, to, pid, fp->frag_num, i, type, NULL, 0, NULL);
	return fp;
}

packet_state_t fragmented_packet_state(struct fragmented_packet *fp, const struct nodeID *from, const struct nodeID *to, struct list_head * requests)
{
	packet_state_t res = PKT_READY;
	frag_id_t i, j, last = 0;

	for(i=0, j=fp->frag_num - 1; i < fp->frag_num; i++)
	{
		if (fp->frags[j-i].data == NULL)
		{
			if (res == PKT_READY)
				res = PKT_LOADING;
			if (last != 0 || j-i == 0)
				frag_request_create(from, to, fp->packet_id, j-i, requests);
		} else
			if (last == 0)
				last = j-i;
	}

	return res;
}

packet_state_t fragmented_packet_write_fragment(struct fragmented_packet *fp, const struct fragment *f, struct list_head * requests)
{
	packet_state_t res = PKT_ERROR;
	struct fragment * nf;
	struct nodeID *from, *to;

	if (fp && f && requests && f->id < fp->frag_num)
	{
		nf = &(fp->frags[f->id]);
		fragment_deinit(nf);
		from = ((struct net_msg*)f)->from;
		to = ((struct net_msg*)f)->to;
		fragment_init(nf, from, to, f->pid, f->frag_num, f->id, f->type, f->data, f->data_size, NULL);
		res = fragmented_packet_state(fp, from, to, requests);
	}
	return res;
}

int8_t fragmented_packet_dump_data(struct fragmented_packet *fp, uint8_t * buff, size_t * size)
{
	int res = 0;
	frag_id_t i;
	size_t datasize = 0;

	for(i = 0; i < fp->frag_num && res < 1; i++)
	{
		if (fp->frags[i].data_size + datasize < *size)
		{
			memmove(buff+datasize, fp->frags[i].data, fp->frags[i].data_size);
			datasize += fp->frags[i].data_size;
		} else
			res = 1;
	}
	*size = datasize;

	return res;
}

struct fragment * fragmented_packet_fragment(struct fragmented_packet *fp, frag_id_t fid)
{
	if (fid < fp->frag_num)
		return &(fp->frags[fid]);
	return NULL;
}
