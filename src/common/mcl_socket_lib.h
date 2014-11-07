/* $Id: mcl_socket_lib.h,v 1.9 2005/05/24 15:43:23 roca Exp $ */
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


/******************************************************************************/
/*
 * Code derived from Richard Stevens's book
 * "UNIX Network Programming, Volume 1, Second Edition: "Networking
 *  APIs: Sockets and XTI", Prentice Hall, 1998, ISBN 0-13-490012-X.
 * Available at URL: http://www.kohala.com/start/unpv12e.html
 */


/**
 * Set the multicast interface where to send or receive multicast traffic.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param af		address familly
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @param verbosity	level of verbosity needed
 * @return
 */
extern INT32	mcast_set_if	(MCL_SOCKET	sockfd,
				 INT32		af,
				 mcl_addr	*ifaddr,
				 const char	*ifname,
				 INT32		verbosity);


/**
 * Set the TTL (with IPv4) or Hop Count (IPv6).
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param af		address familly
 * @param val		new TTL value to set
 * @return
 */
extern INT32	mcast_set_ttl	(MCL_SOCKET	sockfd,
				 INT32		af,
				 INT32		val);


/**
 * Set/unset the loopback mode for multicast traffic.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param af		address familly
 * @param
 * @return
 */
extern INT32	mcast_set_loop	(MCL_SOCKET	sockfd,
				 INT32		af,
				 INT32		onoff);


/**
 * Join a multicast group.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param sa		mcast group address to join
 * @param src_sa	source address in SSM mode. SSM join mode is
 *			used if and only if src_sa != NULL.
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @return
 */
extern INT32	mcast_join	(MCL_SOCKET	sockfd,
				 const sockaddr *sa,
				 mcl_addr	*ifaddr,
				 const char	*ifname);

/**
 * Leave a multicast group.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param sa		socket address
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @return
 */
extern INT32	mcast_leave	(MCL_SOCKET	sockfd,
				 const sockaddr *sa,
				 mcl_addr	*ifaddr,
				 const char	*ifname);

#ifdef SSM
/**
 * Join a multicast group in SSM mode.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param sa		mcast group address to join
 * @param src_sa	source address in SSM mode.
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @return
 */
extern INT32	ssm_mcast_join	(MCL_SOCKET	sockfd,
				 const sockaddr *sa,
				 const sockaddr	*src_sa,
				 mcl_addr	*ifaddr,
				 const char	*ifname,
				 INT32		verbosity);

/**
 * Leave a multicast group in SSM mode.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param sa		socket address
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @return
 */
extern INT32	ssm_mcast_leave	(MCL_SOCKET	sockfd,
				 const sockaddr *sa,
				 const sockaddr	*src_sa,
				 mcl_addr	*ifaddr,
				 const char	*ifname,
				 INT32		verbosity);
#endif

