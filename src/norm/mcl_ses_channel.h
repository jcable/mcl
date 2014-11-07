/* $Id: mcl_ses_channel.h,v 1.4 2004/06/15 15:53:27 roca Exp $ */
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

#ifndef SESSION_CHANNEL_H
#define SESSION_CHANNEL_H

/**
 * Session channel class.
 * Maintains information for the session channel, i.e. everything that
 * is required to send or receive data or control packets.
 *
 * Two sockets are required:
 * 1- the session socket, bound to the public mcast address/port
 * 2- the private socket, bound to a private locally unique port number
 *
 * The SESSION socket is used for:
 *    - reception by a receiver of multicast packets (data or signaling)
 *	sent by the source
 *    - reception by a source of multicast signaling packets sent by a
 *	receiver
 *
 * The PRIVATE socket is used for:
 *    - all unicast or multicast transmissions by a source (data or signaling)
 *    - all unicast or multicast transmissions by a receiver (signaling)
 *    - reception by a source of unicast signaling packets sent by a receiver
 *    - reception by a receiver of unicast signaling response sent by a source
 */
class mcl_ses_channel {
 
public:
	/****** Public Members ************************************************/

	/** The default constructor. */
	mcl_ses_channel ();

	/** The destructor. */
	~mcl_ses_channel ();

	/**
	 * Easy way to know if the session uses a multicast addr or not.
	 * Prefer this function to a call to ses_addr.is_multicast_addr()
	 * since it is faster and simpler.
	 * @return boolean, true if the session addr is a multicast addr
	 */
	bool			is_mcast_session ();

	/**
	 * Perform all the socket level initialization work for both the
	 * private and session sockets.
	 * Uses the various parameters already initialized with default
	 * values or with appropriate mcl_ctl() calls.
	 * @param mclcb
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	sock_init (mcl_cb *const mclcb);

	/**
	 * Close all the opened sockets if any.
	 */
	mcl_error_status	sock_close (mcl_cb *const mclcb);

	/**
	 * Sends a data or control packet on this channel.
	 * Use the ADU destination addr, otherwise the default session address.
	 * Always uses the private socket.
	 * @param mclcb
	 * @param hdr	header pointer
	 * @param hlen	header length
	 * @param du	data unit to send.
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	send_pkt (mcl_cb	*const mclcb,
					norm_common_hdr_t *const hdr,
					INT32		const hlen,
					mcl_du		*const du);

	/**
	 * Sends a data or control packet on this channel.
	 * The destination address is specified and can be either a unicast
	 * or multicast address.
	 * @param mclcb
	 * @param addr	destination address
	 * @param hdr	header pointer
	 * @param hlen	header length
	 * @param du	data unit to send.
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status
	mcl_ses_channel::send_pkt (mcl_cb		*const mclcb,
				   //mcl_addr		*const addr,
				   mcl_addr		const addr,
				   norm_common_hdr_t	*const hdr,
				   INT32		const hlen,
				   mcl_du		*const du);

	/**
	 * Receive a data or control packet on this channel.
	 * Considers both the session and private sockets.
	 * @param mclcb
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	recv_pkt (mcl_cb *const mclcb);


	/****** Public Attributes *********************************************/
	MCL_SOCKET	ses_sock;	// session socket: mcast rx
	MCL_SOCKET	priv_sock;	// private sock: mcast/uni tx, uni rx
	class mcl_addr	ses_addr;	// mcast address on session socket
	char		*mcast_if_name;	// name of mcast interf. to use or NULL
	class mcl_addr	*mcast_if_addr;	// addr of mcast interf. to use or NULL
	class mcl_addr	priv_addr;	// our own addr from which we tx pkts

	UINT8		ttl;		// ttl to use in case of mcast channel
	INT32		socket_size;	// size to use for tx and rx sockets

	bool		can_tx;		// true when initialized for tx
	bool		can_rx;		// true when initialized for rx
 

private:
	/****** Private Members ***********************************************/

#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
	/**
	 * Simulate packets losses randomly
	 * @return	true if packet should be lost, false if pkt can be sent,
	 */
	bool		mcl_ses_channel::sim_random_loss(mcl_cb *const mclcb);
#endif


	/****** Private Attributes ********************************************/
	INT32		is_mcast;	// -1 before first call to
					// is_mcast_session(), 0 or 1 after
	/*
	 * Select specific variables
	 */
	fd_set		fds;		// set of file descript, used by select
	INT32		nfds;		// highest-numbered fd, plus 1
	INT32		n_fd;		// total nb of fd (ie sockets)

#if defined(SIMUL_TX_LOSSES) || defined(SIMUL_RX_LOSSES)
	INT32		simul_losses_state;
#endif
};


/*
 * tx/rx modes
 * differentiate application mode and signalization mode (usually piggy-backed)
 */
#define	MODE_UNI_TX		0x01	/* can send data in unicast */
#define	MODE_MCAST_TX		0x02	/* can send data in multicast */
#define	MODE_UNI_RX		0x04	/* can receive data in unicast */
#define	MODE_MCAST_RX		0x08	/* can receive data in multicast */
#define	MODE_SIG_UNI_TX		0x10	/* can send signaling in unicast */
#define	MODE_SIG_MCAST_TX	0x20	/* can send signaling in unicast */
#define	MODE_SIG_UNI_RX		0x40	/* can recv signaling in unicast */
#define	MODE_SIG_MCAST_RX	0x80	/* can recv signaling in unicast */


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#endif // !SESSION_CHANNEL_H
