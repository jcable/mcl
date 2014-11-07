/* $Id: mcl_socket.h,v 1.5 2005/05/23 11:11:52 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
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

#ifndef MCL_SOCKET_H  /* { */
#define MCL_SOCKET_H


/**
 * This class contains the socket(s) on which transmissions or receptions
 * are performed.
 * Used both for sender and receivers, unicast and multicast.
 * In multicast, there is typically one layer class per socket class.
 * In unicast, there is typically only one layer class, and one socket class.
 */
class mcl_socket {
 
public:

	/****** Public Members ************************************************/

	/**
	 * Constructor.
	 */
	mcl_socket ();

	/**
	 * Destructor.
	 */
	~mcl_socket ();

	/**
	 * Reset the class.
	 * Frees all structures (if any) and resets the various fields.
	 */
	void		reset ();

	/**
	 * Initialize as a sender.
	 * @param mclcb
	 * @param lay_id	Identifier of the socket class. This is
	 * 			an integer in [0; MAX_NB_SOCKETS[. With
	 * 			multicast, it is typically the layer id.
	 * @param ses_addr	the session address (mcast/unicast).
	 * @param ifaddr	address of the mcast interface to use or NULL
	 * @param ifname	name of the mcast interface to use or NULL
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	init_as_sender	(mcl_cb		*const mclcb,
						 INT32		lay_id,
						 mcl_addr	*ses_addr,
						 mcl_addr	*ifaddr,
				 		 char		*ifname);

	/**
	 * Initialize as a receiver.
	 *
	 * @param mclcb
	 * @param lay_id	Identifier of the socket class. This is
	 * 			an integer in [0; MAX_NB_SOCKETS[. With
	 * 			multicast, it is typically the layer id.
	 * @param ses_addr	the session address (mcast/unicast).
	 * @param src_addr	the address of the source, in SSM mode.
	 * @param ifaddr	address of the mcast interface to use or NULL
	 * @param ifname	name of the mcast interface to use or NULL
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	init_as_receiver(mcl_cb		*const mclcb,
						 INT32		lay_id,
						 mcl_addr	*ses_addr,
#ifdef SSM
						 mcl_addr	*src_addr,
#endif
						 mcl_addr	*ifaddr,
				 		 char		*ifname);

	/**
	 * Close the sockets.
	 * Does not free anything (call reset for that).
	 * @param mclcb
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	close_sockets (mcl_cb	*const mclcb);

	/**
	 * Return the socket class id.
	 * @return		Identifier.
	 */
	INT32		get_id ();

	/**
	 * True if transmissions are possible (i.e. if priv_sock has been
	 * fully initialized).
	 * @return		True if transmissions are possible.
	 */
	bool		can_tx ();

	/**
	 * True if receptions are possible (i.e. if ses_sock has been
	 * fully initialized, and with multicast if JOIN has been issued).
	 * @return		True if receptions are possible.
	 */
	bool		can_rx ();


	/****** Public Attributes *********************************************/

	/**
	 * Session socket used for receptions.
	 * It is bind() to the session address and port number.
	 */
	MCL_SOCKET	ses_sock;

	/**
	 * Private socket used for transmissions.
	 * This socket is required to benefit from a private local port
	 * number. It fully identifies the source of the packets.
	 */
	MCL_SOCKET	priv_sock;

	/**
	 * Address used on the session socket.
	 */
	class mcl_addr	addr;

	/**
	 * Address info associated to addr.
	 * Structure returned by getaddrinfo() syscall.
	 */
	struct addrinfo	*addrinfo;


private:

	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/

	/**
	 * Identifier of this mcl_socket class.
	 */
 	INT32		id;

	/**
	 * State of the transmission socket (if any).
	 */
	enum mcl_socket_tx_state {
		SOCK_TX_INVALID,
		SOCK_TX_INITIALIZED
	};
	mcl_socket_tx_state	tx_state;

	/**
	 * State of the reception socket (if any).
	 */
	enum mcl_socket_rx_state {
		SOCK_RX_INVALID,
		SOCK_RX_INITIALIZED,	// initialized and joined
		SOCK_RX_NOT_JOINED	// initialized but no longer joined
					// (drop_membership has been issued)
	};
	mcl_socket_rx_state	rx_state;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline bool
mcl_socket::can_tx ()
{
	return (this->tx_state == SOCK_TX_INITIALIZED);
}

inline bool
mcl_socket::can_rx ()
{
	return (this->rx_state == SOCK_RX_INITIALIZED);
}

#endif /* } MCL_SOCKET_H */
