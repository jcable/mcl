/* $Id: mcl_rx.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_RX_H
#define MCL_RX_H

/**
 * Reception class.
 */
class mcl_rx {
  
public:
	/****** Public Members ************************************************/
	mcl_rx ();
	~mcl_rx ();

	/**
	 * Set the highest adu seq number of this session as announced
	 * with NO_NEW_OBJECT.
	 * @return	highest adu seq number or 0 if unknown.
	 */
	void		set_highest_adu_seq_of_session (UINT32 max_id);

	/**
	 * Return the highest adu seq number of this session if any.
	 * @return	highest adu seq number or 0 if unknown.
	 */
	UINT32		get_highest_adu_seq_of_session () const;

	/**
	 * Process the incoming packet just received.
	 * This can be a pure signaling packet or a pure data packet, or
	 * a data packet plus a signaling header (usual case).
	 * This function and the following ones take control of the pbuf
	 * buffer, and can free it when required...
	 * This function is used both by a sender (for feedback packets)
	 * and a receiver (data and control packets).
	 */
	void	process_pkt	(mcl_cb *const mclcb, class mcl_rx_pkt *pkt,
				 mcl_addr *saddr);

	/**
	 * Try to return an ADU to the appli.
	 * Maybe there is a gap and this is not possible...
	 * Return the amount of data copied to userbuf, 0 if some
	 * data is available but not copied to userbuf, and < 0 if
	 * no data is available.
	 */
	INT32	return_adu_to_appli(mcl_cb *const mclcb,
				    char *userbuf, INT32 userlen,
				    struct sockaddr *saddr, INT32 *saddr_len);

	/****** Public Attributes *********************************************/

 
private:
	/****** Private Members ***********************************************/

	/**
	 * Process an incoming NORM_DATA packet.
	 * @param mclcb
	 * @param pkt		pointer to packet
	 * @param saddr		source address for this packet
	 * @param chdr_infos	pointer to the common_infos struct
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  process_data_pkt (mcl_cb		*const mclcb,
					class mcl_rx_pkt	*pkt,
					mcl_addr		*saddr,
					mcl_common_hdr_infos_t	*chdr_infos);

#if 0
	/**
	 * Process an incoming NORM_CMD packet.
	 * @param mclcb
	 * @param pkt		pointer to packet
	 * @param saddr		source address for this packet
	 * @param chdr_infos	pointer to the common_infos struct
	 */
	mcl_error_status  process_cmd_pkt(mcl_cb *const mclcb,
				 class mcl_rx_pkt *pkt,
				 mcl_addr *saddr,
				 mcl_common_hdr_infos_t *chdr_infos);
#endif


	/****** Private Attributes ********************************************/
 
	UINT32		max_adu_seq; // highest ADU seq nb

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline void
mcl_rx::set_highest_adu_seq_of_session (UINT32 max_id)
{
	this->max_adu_seq = max_id;
}

inline UINT32	
mcl_rx::get_highest_adu_seq_of_session () const
{
	return this->max_adu_seq;
}

#endif // MCL_RX_H
