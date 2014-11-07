/* $Id: mcl_rx_storage.h,v 1.3 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_RX_STORAGE
#define MCL_RX_STORAGE


/**
 * Class that stores all the incoming packets.
 */
class mcl_rx_pkt {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 * Allocates the data buffer with the size provided.
	 * @param max_size	the maximum packet size, including headers
	 */
	mcl_rx_pkt (INT32 max_size);

	/**
	 * Default destructor.
	 * This destructor deletes both the pkt and the buffer, but NOT the DU
	 * (if any).
	 */
	~mcl_rx_pkt ();

	/**
	 * Return a pointer to the buffer where the whole packet is stored
	 * @return	pointer to buffer.
	 */
	char	*get_buf ();

	/**
	 * Return the buffer length.
	 * The actual size of the packet stored is of course inferior or
	 * equal to this size.
	 * @return	buffer legnth.
	 */
	INT32	get_buf_len ();

	/****** Public Attributes *********************************************/

	/**
	 * Return the whole packet length, including headers.
	 */
	INT32		pkt_len;


private:
	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/

	/**
	 * Buffer where packet is stored.
	 * This buffer is allocated once, at creation.
	 */
	char	*buf;

	/**
	 * Length of this buffer.
	 */
	INT32	buf_len;
};


/**
 * Class explanation
 */
class mcl_rx_storage {
 
public:
	/****** Public Members ************************************************/
	mcl_rx_storage();
	~mcl_rx_storage();

#if 0
	/****** Public Attributes *********************************************/
  
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
#endif // 0
 
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline char *
mcl_rx_pkt::get_buf ()
{
	return this->buf;
}

inline INT32
mcl_rx_pkt::get_buf_len ()
{
	return this->buf_len;
}


#endif // MCL_RX_STORAGE
