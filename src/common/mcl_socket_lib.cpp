/* $Id: mcl_socket_lib.cpp,v 1.15 2005/05/24 15:43:23 roca Exp $ */
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

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
#endif // RM_PROTOCOL


/******************************************************************************/
/*
 * Code derived from Richard Stevens's book:
 * "UNIX Network Programming, Volume 1, Second Edition: "Networking
 *  APIs: Sockets and XTI", Prentice Hall, 1998, ISBN 0-13-490012-X.
 * Available at URL: http://www.kohala.com/start/unpv12e.html
 */


/**
 * Set the multicast interface where to send or receiver multicast traffic.
 * => See header file for more informations.
 */
INT32
mcast_set_if	(MCL_SOCKET	sockfd,
		 INT32		af,
		 mcl_addr	*ifaddr,
		 const char	*ifname,
		 INT32		verbosity)
{
	INT32	err;

	if (verbosity >= 4) {
		PRINT_OUT((mcl_stdout,
			"mcast_set_if: af=%d, ifaddr=%s, ifname=%s\n", af,
			(ifaddr != NULL) ? ifaddr->get_addr_string() : "null",
			(ifname != NULL) ? ifname : "null"))
	}
	switch (af) {
	case AF_INET: {
		struct in_addr		inaddr;
		struct sockaddr_in	sin;	// used if ifaddr is provided
#ifndef WIN32	/* SIOCGIFADDR not defined in WinSock2? */
		struct ifreq		ifreq;	// used if ifname is provided

		if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IF_NAMESIZE);
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
				/* i/f name not found */
				perror("mcast_set_if: ERROR, ioctl(SIOCGIFADDR)");
				PRINT_ERR((mcl_stderr,
					"mcast_set_if: ERROR, ioctl(SIOCGIFADDR) failed for ifname %s\n",
					ifname))
				errno = ENXIO;
				return(-1);
			}
			memcpy(&inaddr,
				&((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
				sizeof(struct in_addr));
		} else
#endif /* OS */
		if (ifaddr != NULL) {
			if (ifaddr->is_ipv4_addr() == false) {
				PRINT_ERR((mcl_stderr,
				"mcast_set_if: ERROR, ifaddr is IPv6 whereas the declared address family is IPv4!\n"))
				errno = EINVAL;
				return(-1);
			}
			ifaddr->get_addr_struct(&sin);
			memcpy(&inaddr, &((struct sockaddr_in*)&sin)->sin_addr,
				sizeof(struct in_addr));
		} else {
			/* Remove previous. Set interface to default */
			inaddr.s_addr = htonl(INADDR_ANY);
		}
		if ((err = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF,
				(char*)&inaddr, sizeof(struct in_addr))) < 0) {
			int	e = errno;
			perror("mcast_set_if: ERROR, setsockopt(IP_MULTICAST_IF)");
			errno = e;
			return(-1);
		}
		return (err);
	}
#ifdef INET6
	case AF_INET6: {
		u_int	index;

#ifndef WIN32
		if (ifname == NULL) {
			/* must supply name */
			PRINT_ERR((mcl_stderr,
				"mcast_set_if: ERROR, ifname required\n"))
			errno = EINVAL;
			return(-1);
		}		
		if ((index = if_nametoindex(ifname)) == 0) {
			/* i/f name not found */
			perror("mcast_set_if: ERROR, if_nametoindex");
			errno = ENXIO;
			return(-1);
		}
#else
		index = 0;
#endif /* OS */
		if ((err = setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
				(char*)&index, sizeof(index))) < 0) {
			int	e = errno;
			perror("mcast_set_if: ERROR, setsockopt(IP_MULTICAST_IF)");
			errno = e;
			return(-1);
		}
		return (err);
	}
#endif
	default:
		PRINT_ERR((mcl_stderr,
		"mcast_set_if: ERROR, unsupported address family %d\n", af))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}


/**
 * Set the TTL (with IPv4) or Hop Count (IPv6).
 * => See header file for more informations.
 */
INT32
mcast_set_ttl	(MCL_SOCKET	sockfd,
		 INT32		af,
		 INT32		val)
{
	INT32	err;

	switch (af) {
	case AF_INET: {
#ifdef WIN32
		UINT32	ttl;
#else
		/* solaris seems to care about the int/char type, not linux! */
		UINT8	ttl;
#endif

		ttl = val;
		if ((err = setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
					(char*)&ttl, sizeof(ttl))) < 0) {
			perror("mcast_set_ttl: ERROR, setsockopt(TTL)");
		}
		break;
		}
#ifdef INET6
	case AF_INET6: {
		INT32	hops;
		hops = val;
		if ((err = setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
					(char*)&hops, sizeof(hops))) < 0) {
			perror("mcast_set_ttl: ERROR, setsockopt(HOPS)");
		}
		break;
		}
#endif
	default:
		PRINT_ERR((mcl_stderr,
		"mcast_set_ttl: ERROR, unsupported address family %d\n", af))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
	return err;
}


/**
 * Set/unset the loopback mode for multicast traffic.
 * => See header file for more informations.
 */
INT32
mcast_set_loop	(MCL_SOCKET	sockfd,
		 INT32		af,
		 INT32		onoff)
{
	switch (af) {
	case AF_INET: {
		UINT8	flag;
		flag = onoff;
		return(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
				(char*)&flag, sizeof(flag)));
		}
#ifdef INET6
	case AF_INET6: {
		UINT32	flag;
		flag = onoff;
		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
				(char*)&flag, sizeof(flag)));
		}
#endif
	default:
		PRINT_ERR((mcl_stderr,
		"mcast_set_loop: ERROR, unsupported address family %d\n", af))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}


/**
 * Join a multicast group.
 * => See header file for more informations.
 */
INT32
mcast_join (MCL_SOCKET		sockfd,
	    const sockaddr	*sa,
	    mcl_addr		*ifaddr,
	    const char		*ifname)
{
	INT32	err;		// return code

	switch (sa->sa_family) {
	case AF_INET: {
		struct ip_mreq		mreq;
		struct sockaddr_in	sin;	// used if ifaddr is provided
#ifndef WIN32	/* SIOCGIFADDR not defined in WinSock2? */
		struct ifreq		ifreq;
#endif /* OS */

		memcpy(&mreq.imr_multiaddr,
			   &((struct sockaddr_in *) sa)->sin_addr,
			   sizeof(struct in_addr));
#ifndef WIN32
		if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
				PRINT_ERR((mcl_stderr,
				"mcast_join: ERROR, SIOCGIFADDR failed\n"))
				return(-1);
			}
			memcpy(&mreq.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
#endif /* OS */
		if (ifaddr != NULL) {
			if (ifaddr->is_ipv4_addr() == false) {
				PRINT_ERR((mcl_stderr,
				"mcast_join: ERROR, ifaddr is IPv6 whereas the declared address family is IPv4!\n"))
				errno = EINVAL;
				return(-1);
			}
			ifaddr->get_addr_struct(&sin);
			memcpy(&mreq.imr_interface,
				&((struct sockaddr_in*)&sin)->sin_addr,
				sizeof(struct in_addr));
		} else {
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		}
		if ((err = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				  (char*)&mreq, sizeof(mreq))) < 0) {
#ifndef WIN32
			perror("mcast_join: IP_ADD_MEMBERSHIP:");
#endif
			PRINT_ERR((mcl_stderr,
			"mcast_join: ERROR, IP_ADD_MEMBERSHIP failed\n"))
		}
		return err;
		}

#ifdef INET6
	case AF_INET6: {
		struct ipv6_mreq	mreq6;

		memcpy(&mreq6.ipv6mr_multiaddr,
			   &((struct sockaddr_in6 *) sa)->sin6_addr,
			   sizeof(struct in6_addr));
#ifndef WIN32
		if (ifname != NULL) {
			if ((mreq6.ipv6mr_interface = if_nametoindex(ifname)) == 0) {
				errno = ENXIO;	/* i/f name not found */
				return(-1);
			}
		} else {
			mreq6.ipv6mr_interface = 0;
		}
#else /* WIN32 */
		 mreq6.ipv6mr_interface = 0;
#endif /* OS */

#ifdef FREEBSD
		err = setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
				  (char*)&mreq6, sizeof(mreq6));
#else
		err = setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
				  (char*)&mreq6, sizeof(mreq6));
#endif /* OS */
		if (err < 0)  {
#ifndef WIN32
			perror("mcast_join: IPV6_ADD_MEMBERSHIP:");
#endif
			PRINT_ERR((mcl_stderr,
			"mcast_join: ERROR, IPV6_ADD_MEMBERSHIP failed\n"))
		}
		return err;
		}
#endif /* INET6 */

	default:
		PRINT_ERR((mcl_stderr,
			"mcast_join: ERROR, unsupported address family %d\n",
			sa->sa_family))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}


/**
 * Leave a multicast group.
 * => See header file for more informations.
 */
INT32
mcast_leave (MCL_SOCKET		sockfd,
	    const sockaddr	*sa,
	    mcl_addr		*ifaddr,
	    const char		*ifname)
{
	switch (sa->sa_family) {
	case AF_INET: {
		struct ip_mreq		mreq;
		struct sockaddr_in	sin;	// used if ifaddr is provided
#ifndef WIN32	/* SIOCGIFADDR not defined in WinSock2? */
		struct ifreq		ifreq;
#endif /* OS */

		memcpy(&mreq.imr_multiaddr,
			   &((struct sockaddr_in *) sa)->sin_addr,
			   sizeof(struct in_addr));
#ifndef WIN32
		if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
				return(-1);
			}
			memcpy(&mreq.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
#endif /* OS */
		if (ifaddr != NULL) {
			if (ifaddr->is_ipv4_addr() == false) {
				PRINT_ERR((mcl_stderr,
				"mcast_leave: ERROR, ifaddr is IPv6 whereas the declared address family is IPv4!\n"))
				errno = EINVAL;
				return(-1);
			}
			ifaddr->get_addr_struct(&sin);
			memcpy(&mreq.imr_interface,
				&((struct sockaddr_in*)&sin)->sin_addr,
				sizeof(struct in_addr));
		} else
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		return(setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
				  (char*)&mreq, sizeof(mreq)));
		}

#ifdef INET6
	case AF_INET6: {
		struct ipv6_mreq	mreq6;

		memcpy(&mreq6.ipv6mr_multiaddr,
			   &((struct sockaddr_in6 *) sa)->sin6_addr,
			   sizeof(struct in6_addr));
#ifndef WIN32
		if (ifname != NULL) {
			if ( (mreq6.ipv6mr_interface = if_nametoindex(ifname)) == 0) {
				errno = ENXIO;	/* i/f name not found */
				return(-1);
			}
		} else {
			mreq6.ipv6mr_interface = 0;
		}
#else /* WIN32 */
		 mreq6.ipv6mr_interface = 0;
#endif /* OS */
	
#ifdef FREEBSD
		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
				  (char*)&mreq6, sizeof(mreq6)));
#else
		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
				  (char*)&mreq6, sizeof(mreq6)));
#endif /* OS */
		}
#endif /* INET6 */

	default:
		PRINT_ERR((mcl_stderr,
			"mcast_leave: ERROR, unsupported address family %d\n",
			sa->sa_family))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}


#ifdef SSM
/**
 * Join a multicast group in SSM mode.
 * => See header file for more informations.
 */
INT32
ssm_mcast_join (MCL_SOCKET	sockfd,
		const sockaddr	*sa,
		const sockaddr	*src_sa,
		mcl_addr	*ifaddr,
		const char	*ifname,
		INT32		verbosity)
{
	INT32	err;		// return code

	ASSERT(src_sa != NULL);

	if (verbosity >= 4) {
		mcl_addr	tmp_sa;
		mcl_addr	tmp_src_sa;

		tmp_sa.set_addr_struct(sa);
		tmp_src_sa.set_addr_struct(src_sa);
		/* get_addr_string uses a static buf, so split the PRINT_OUT */
		PRINT_OUT((mcl_stdout,
			"ssm_mcast_join: sa=%s, ",
			tmp_sa.get_addr_string()))
		PRINT_OUT((mcl_stdout,
			"src=%s, ",
			tmp_src_sa.get_addr_string()))
		PRINT_OUT((mcl_stdout,
			"ifaddr=%s, ifname=%s\n",
			(ifaddr != NULL) ? ifaddr->get_addr_string() : "null",
			(ifname != NULL) ? ifname : "null"))
	}
	switch (sa->sa_family) {
	case AF_INET: {
		struct ip_mreq_source	mreq_s;	// for IGMPv3 SSM join
		struct sockaddr_in	sin;	// used if ifaddr is provided
#ifndef WIN32	/* SIOCGIFADDR not defined in WinSock2? */
		struct ifreq		ifreq;
#endif /* OS */

#ifdef DEBUG
		/* sanity checks */
		if (((ifaddr != NULL) && (ifaddr->is_ipv4_addr() == false)) ||
		    (src_sa->sa_family != AF_INET)) {
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_join: ERROR, ifaddr or src_addr is IPv6 whereas the declared address family is IPv4!\n"))
			errno = EINVAL;
			return(-1);
		}
#endif /* DEBUG */
		memcpy(&mreq_s.imr_multiaddr,
			   &((struct sockaddr_in *) sa)->sin_addr,
			   sizeof(struct in_addr));
#ifndef WIN32
		if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
				PRINT_ERR((mcl_stderr,
				"ssm_mcast_join: ERROR, SIOCGIFADDR failed\n"))
				return(-1);
			}
			memcpy(&mreq_s.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
#endif /* OS */
		if (ifaddr != NULL) {
			ifaddr->get_addr_struct(&sin);
			memcpy(&mreq_s.imr_interface,
				&((struct sockaddr_in*)&sin)->sin_addr,
				sizeof(struct in_addr));
		} else {
			mreq_s.imr_interface.s_addr = htonl(INADDR_ANY);
		}
		memcpy(&mreq_s.imr_sourceaddr,
			   &((struct sockaddr_in *) src_sa)->sin_addr,
			   sizeof(struct in_addr));
		if ((err = setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
			  (char*)&mreq_s, sizeof(mreq_s))) < 0) {
#ifndef WIN32
			perror("ssm_mcast_join: IP_ADD_SOURCE_MEMBERSHIP:");
#endif
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_join: ERROR, IP_ADD_SOURCE_MEMBERSHIP failed\n"))
		}
		return err;
		}

#ifdef INET6
#ifdef LINUX	// only available on Linux in this release!
	case AF_INET6: {
		struct group_source_req		gs_req;	// for MLDv2 SSM join

#ifdef DEBUG
		/* sanity checks */
		if (src_sa->sa_family != AF_INET6) {
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_join: ERROR, src_addr is IPv4 whereas the declared address family is IPv6!\n"))
			errno = EINVAL;
			return(-1);
		}
#endif /* DEBUG */
		memcpy(&gs_req.gsr_group, (void*)sa,
			sizeof(struct sockaddr_in6));
#ifndef WIN32
		if (ifname != NULL) {
			if ((gs_req.gsr_interface = if_nametoindex(ifname)) == 0) {
				errno = ENXIO;	/* i/f name not found */
				return(-1);
			}
		} else {
			gs_req.gsr_interface = 0;
		}
#else /* WIN32 */
		 gs_req.gsr_interface = 0;
#endif /* OS */
		memcpy(&gs_req.gsr_source, (void*)src_sa,
			sizeof(struct sockaddr_in6));
		if ((err = setsockopt(sockfd, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP,
				  (char*)&gs_req, sizeof(gs_req))) < 0) {
#ifndef WIN32
			perror("ssm_mcast_join: MCAST_JOIN_SOURCE_GROUP for IPv6:");
#endif
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_join: ERROR, MCAST_JOIN_SOURCE_GROUP for IPv6failed\n"))
		}
		return err;
		}
#endif /* LINUX */
#endif /* INET6 */

	default:
		PRINT_ERR((mcl_stderr,
			"ssm_mcast_join: ERROR, unsupported address family %d\n",
			sa->sa_family))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}


/**
 * Leave a multicast group in SSM mode.
 * IPv4 and IPv6 compatible.
 * @param sockfd	socket file descriptor
 * @param sa		socket address
 * @param ifaddr	interface address. Only used for IPv4.
 * @param ifname	interface name (e.g. eth0). For IPv4 or IPv6.
 * @return
 */
extern INT32
ssm_mcast_leave	(MCL_SOCKET	sockfd,
		 const sockaddr *sa,
		 const sockaddr	*src_sa,
		 mcl_addr	*ifaddr,
		 const char	*ifname,
		 INT32		verbosity)
{
	INT32	err;		// return code

	if (verbosity >= 4) {
		mcl_addr	tmp_sa;
		mcl_addr	tmp_src_sa;

		tmp_sa.set_addr_struct(sa);
		tmp_src_sa.set_addr_struct(src_sa);
		/* get_addr_string uses a static buf, so split the PRINT_OUT */
		PRINT_OUT((mcl_stdout,
			"ssm_mcast_leave: sa=%s, ",
			tmp_sa.get_addr_string()))
		PRINT_OUT((mcl_stdout,
			"src=%s, ",
			tmp_src_sa.get_addr_string()))
		PRINT_OUT((mcl_stdout,
			"ifaddr=%s, ifname=%s\n",
			(ifaddr != NULL) ? ifaddr->get_addr_string() : "null",
			(ifname != NULL) ? ifname : "null"))
	}
	switch (sa->sa_family) {
	case AF_INET: {
		struct ip_mreq_source	mreq_s;	// for IGMPv3 SSM join
		struct sockaddr_in	sin;	// used if ifaddr is provided
#ifndef WIN32	/* SIOCGIFADDR not defined in WinSock2? */
		struct ifreq		ifreq;
#endif /* OS */

		memcpy(&mreq_s.imr_multiaddr,
			   &((struct sockaddr_in *) sa)->sin_addr,
			   sizeof(struct in_addr));
#ifndef WIN32
		if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) {
				return(-1);
			}
			memcpy(&mreq_s.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
#endif /* OS */
		if (ifaddr != NULL) {
			if (ifaddr->is_ipv4_addr() == false) {
				PRINT_ERR((mcl_stderr,
				"ssm_mcast_leave: ERROR, ifaddr is IPv6 whereas the declared address family is IPv4!\n"))
				errno = EINVAL;
				return(-1);
			}
			ifaddr->get_addr_struct(&sin);
			memcpy(&mreq_s.imr_interface,
				&((struct sockaddr_in*)&sin)->sin_addr,
				sizeof(struct in_addr));
		} else
			mreq_s.imr_interface.s_addr = htonl(INADDR_ANY);

		memcpy(&mreq_s.imr_sourceaddr,
			   &((struct sockaddr_in *) src_sa)->sin_addr,
			   sizeof(struct in_addr));
		if ((err = setsockopt(sockfd, IPPROTO_IP,
					IP_DROP_SOURCE_MEMBERSHIP,
					(char*)&mreq_s, sizeof(mreq_s))) < 0) {
#ifndef WIN32
			perror("ssm_mcast_leave: IP_DROP_SOURCE_MEMBERSHIP for IPv4:");
#endif
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_leave: ERROR, IP_DROP_SOURCE_MEMBERSHIP for IPv4 failed\n"))
		}
		return err;
		}

#ifdef INET6
#ifdef LINUX	// only available on Linux in this release!
	case AF_INET6: {
		struct group_source_req		gs_req;	// for MLDv2 SSM leave

		memcpy(&gs_req.gsr_group, (void*)sa,
			sizeof(struct sockaddr_in6));
#ifndef WIN32
		if (ifname != NULL) {
			if ( (gs_req.gsr_interface = if_nametoindex(ifname)) == 0) {
				errno = ENXIO;	/* i/f name not found */
				return(-1);
			}
		} else {
			gs_req.gsr_interface = 0;
		}
#else /* WIN32 */
		 gs_req.gsr_interface = 0;
#endif /* OS */
		memcpy(&gs_req.gsr_source, (void*) src_sa,
			sizeof(struct sockaddr_in6));
		if ((err = setsockopt(sockfd, IPPROTO_IPV6, MCAST_LEAVE_SOURCE_GROUP,
				  (char*)&gs_req, sizeof(gs_req))) < 0) {
#ifndef WIN32
			perror("ssm_mcast_leave: MCAST_LEAVE_SOURCE_GROUP for IPv6:");
#endif
			PRINT_ERR((mcl_stderr,
			"ssm_mcast_leave: ERROR, MCAST_LEAVE_SOURCE_GROUP for IPv6failed\n"))
		}
		return err;
		}
#endif /* LINUX */
#endif /* INET6 */

	default:
		PRINT_ERR((mcl_stderr,
			"ssm_mcast_leave: ERROR, unsupported address family %d\n",
			sa->sa_family))
#ifndef WIN32
		errno = EPROTONOSUPPORT;
#endif  // !WIN32
		return(-1);
	}
}
#endif /* SSM */

