/* $Id: mcl_rx_storage.cpp,v 1.2 2004/01/30 16:27:43 roca Exp $ */
/*
 *  Copyright (c) 2004 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include "mcl_includes.h"

/**
 * Default constructor.
 * Allocates the data buffer with the size provided.
 * @param max_size	the maximum packet size, including headers
 */
mcl_rx_pkt::mcl_rx_pkt (INT32 max_size)
{
	//ASSERT(max_size > (INT32)sizeof(norm_common_hdr_t));
	this->buf = new char[max_size];
	ASSERT(this->buf);
	this->buf_len = max_size;
	this->pkt_len = 0;
}


/**
 * Default destructor.
 * This destructor deletes both the pkt and the buffer, but NOT the DU
 * (if any).
 */
mcl_rx_pkt::~mcl_rx_pkt ()
{
	if (this->buf) {
		delete [] this->buf;
		this->buf = NULL;
	}
	this->buf_len = 0;
	this->pkt_len = 0;
}


mcl_rx_storage::mcl_rx_storage()
{
}

mcl_rx_storage::~mcl_rx_storage()
{
}

