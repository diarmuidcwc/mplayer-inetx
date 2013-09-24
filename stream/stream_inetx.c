/*
 * stream layer for MPEG over UDP, based on previous work from Dave Chapman
 *
 * Copyright (C) 2006 Benjamin Zores
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mp_msg.h"
#include "network.h"
#include "stream.h"
#include "url.h"
#include "udp.h"


static int
inetx_streaming_read_trunc( int fd, char *buffer, int size, streaming_ctrl_t *stream_ctrl ) {
	const unsigned int INETX_HDR_LEN = 28;
	int len = nop_streaming_read(fd,buffer,size-INETX_HDR_LEN,stream_ctrl);
	memmove( buffer,buffer+INETX_HDR_LEN,len-INETX_HDR_LEN);
	return len-INETX_HDR_LEN;
}

inetx_streaming_start (stream_t *stream)
{
  streaming_ctrl_t *streaming_ctrl;
  int fd;

  if (!stream)
    return -1;

  streaming_ctrl = stream->streaming_ctrl;
  fd = stream->fd;

  if (fd < 0)
  {
    fd = udp_open_socket (streaming_ctrl->url);
    if (fd < 0)
      return -1;
    stream->fd = fd;
  }

  streaming_ctrl->streaming_read = inetx_streaming_read_trunc;
  streaming_ctrl->streaming_seek = nop_streaming_seek;
  streaming_ctrl->prebuffer_size = 64 * 1024; /* 64 KBytes */
  streaming_ctrl->buffering = 0;
  streaming_ctrl->status = streaming_playing_e;

  return 0;
}


static int
inetx_stream_open (stream_t *stream, int mode, void *opts, int *file_format)
{
  mp_msg (MSGT_OPEN, MSGL_INFO, "STREAM_INETX, URL: %s\n", stream->url);
  stream->streaming_ctrl = streaming_ctrl_new ();
  if (!stream->streaming_ctrl)
    return STREAM_ERROR;

  stream->streaming_ctrl->bandwidth = network_bandwidth;
  stream->streaming_ctrl->url = url_new(stream->url);

  
  
  //const unsigned int INETX_HDR_LEN = 28;
  //mp_msg(MSGT_OPEN,MSGL_INFO,"STREAM_INETX start byte position : 0x%X\n",INETX_HDR_LEN);
  //stream->start_pos=INETX_HDR_LEN;
  
  if (stream->streaming_ctrl->url->port == 0)
  {
    mp_msg (MSGT_NETWORK, MSGL_ERR,
            "You must enter a port number for INETX streams!\n");
    streaming_ctrl_free (stream->streaming_ctrl);
    stream->streaming_ctrl = NULL;

    return STREAM_UNSUPPORTED;
  }

  if (inetx_streaming_start (stream) < 0)
  {
    mp_msg (MSGT_NETWORK, MSGL_ERR, "inetx_streaming_start failed\n");
    streaming_ctrl_free (stream->streaming_ctrl);
    stream->streaming_ctrl = NULL;

    return STREAM_UNSUPPORTED;
  }

  stream->type = STREAMTYPE_STREAM;
  fixup_network_stream_cache (stream);

  return STREAM_OK;
}

const stream_info_t stream_info_inetx = {
  "MPEG over iNET-X streaming",
  "inetx",
  "Dave Chapman, Benjamin Zores, Diarmuid Collins",
  "native inetx support",
  inetx_stream_open,
  { "inetx", NULL},
  NULL,
  0 // Urls are an option string
};
