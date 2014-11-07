/* $Id: mcl_addr.cpp,v 1.18 2005/05/24 15:43:22 roca Exp $ */
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


mcl_addr::mcl_addr()
{
	/*
	 * Clear the address structure and set an IPv4 address family.
	 */
	memset(&(this->inet_address), 0, sizeof(this->inet_address));
	this->inet_address.sin.sin_family = AF_INET;
	this->is_multicast = false;
	this->is_inet6 = false;
}


mcl_addr::mcl_addr(bool	is_ipv4)
{
	/*
	 * Clear the address structure and set the address family.
	 */
	memset(&(this->inet_address), 0, sizeof(this->inet_address));
	if (is_ipv4) {
		this->inet_address.sin.sin_family = AF_INET;
		this->is_multicast = false;
		this->is_inet6 = false;
	}
#ifdef INET6
	else {
		this->inet_address.sin6.sin6_family = AF_INET6;
		this->is_multicast = false;
		this->is_inet6 = true;
	}
#endif /* INET6 */
}


mcl_addr::mcl_addr(const mcl_addr& address)
{
	/*
	 * Copy the address structure.
	 */
#ifdef INET6
	if (address.is_ipv6_addr())
		this->inet_address.sin6 = address.inet_address.sin6;
	else
#endif /* INET6 */
		this->inet_address.sin = address.inet_address.sin;
	this->is_multicast = address.is_multicast;
	this->is_inet6 = address.is_inet6;
}


mcl_addr&
mcl_addr::operator=(const mcl_addr& address)
{
	if (&address == NULL) return (*this);
	else if (this != &address) {
		/* Copy the address structure */
#ifdef INET6
		if (address.is_ipv6_addr())
			this->inet_address.sin6 =
					address.inet_address.sin6;
		else
#endif /* INET6 */
			this->inet_address.sin =
					address.inet_address.sin;
		this->is_multicast = address.is_multicast;
		this->is_inet6 = address.is_inet6;
	}
	return(*this);
}


mcl_addr::~mcl_addr()
{
	/*
	 * Do nothing.
	 */
}


/****** SET FUNCTIONS *************************************************/

void
mcl_addr::reset()
{
	/*
	 * Clear the address structure and set the address family.
	 */
	memset(&(this->inet_address), 0, sizeof(this->inet_address));
	this->inet_address.sin.sin_family = AF_INET;
	this->is_multicast = false;
	this->is_inet6 = false;
}


void
mcl_addr::set_port(UINT16 port)
{
	/*
	 * Store the port.
	 * NB: the sin_port field is at the same place in sockaddr_in and
	 * sockaddr_in6 structures. No need to differentiate...
	 */
	this->inet_address.sin.sin_port = htons(port);
}


void
mcl_addr::set_any_port()
{
	/*
	 * Store the port number 0.
	 * NB: the sin_port field is at the same place in sockaddr_in and
	 * sockaddr_in6 structures. No need to differentiate...
	 */
	this->inet_address.sin.sin_port = htons(0);
}


void
mcl_addr::set_addr(UINT32 address)
{
	/*
	 * Store the address. Limited to IPv4.
	 */
#ifdef INET6
	if (this->is_ipv6_addr()) {
		PRINT_ERR((mcl_stderr,
		"mcl_addr::set_addr: ERROR, IPv4 function called for IPv6 addr.\n"))
		mcl_exit(-1);
	}
#endif /* INET6 */
	this->inet_address.sin.sin_addr.s_addr = htonl(address);
	if ((ntohl(this->inet_address.sin.sin_addr.s_addr) >> 24) >= 224) {
		this->is_multicast = true;
	} else {
		this->is_multicast = false;
	}
}


#ifdef INET6
void
mcl_addr::set_addr(struct in6_addr *address)
{
	/*
	 * Store the address. Limited to IPv6.
	 */
	if (this->is_ipv6_addr() == false) {
		PRINT_ERR((mcl_stderr,
		"mcl_addr::set_addr: ERROR, IPv6 function called for IPv4 addr.\n"))
		mcl_exit(-1);
	}
	memcpy((void*)(&(this->inet_address.sin6.sin6_addr)), (void*)address,
		sizeof(struct in6_addr));
	if (IN6_IS_ADDR_MULTICAST(address)) {
		this->is_multicast = true;
	} else {
		this->is_multicast = false;
	}
}
#endif /* INET6 */


void
mcl_addr::set_any_addr()
{
	/*
	 * Store the address IN(6)ADDR_ANY.
	 */
#ifdef INET6
	if (this->is_ipv6_addr()) {
		memcpy(&(this->inet_address.sin6.sin6_addr), &(in6addr_any),
			sizeof(struct in6_addr));
	} else
#endif /* INET6 */
	{
		this->inet_address.sin.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	this->is_multicast = false;
}


/**
 * Set the entire IPv4 or IPv6 address struct at one time.
 */
mcl_error_status
mcl_addr::set_addr_struct (const struct sockaddr	*address)
{
	switch(address->sa_family) {
	case AF_INET:
		this->set_addr_struct((struct sockaddr_in*)address);
		return MCL_OK;
#ifdef INET6
	case AF_INET6:
		this->set_addr_struct((struct sockaddr_in6*)address);
		return MCL_OK;
#endif /* INET6 */
	default:
		PRINT_ERR((mcl_stderr,
			"mcl_addr::set_addr_struct: ERROR, unsupported address family %d\n",
			address->sa_family))
		return MCL_ERROR;
	}
}


void
mcl_addr::set_addr_struct(const struct sockaddr_in *address)
{
	/*
	 * Get the address structure from address.
	 * Do this safely, copying only the required information.
	 */
	this->inet_address.sin.sin_port		= address->sin_port;
	this->inet_address.sin.sin_addr.s_addr	= address->sin_addr.s_addr;
	this->inet_address.sin.sin_family	= AF_INET;
	if ((ntohl(this->inet_address.sin.sin_addr.s_addr) >> 24) >= 224) {
		this->is_multicast = true;
	} else {
		this->is_multicast = false;
	}
	this->is_inet6 = false;
}


#ifdef INET6
void
mcl_addr::set_addr_struct(const struct sockaddr_in6	*address)
{
	/*
	 * Get the address structure from address.
	 * Do this safely, copying only the required information.
	 */
	this->inet_address.sin6.sin6_port	= address->sin6_port;
	this->inet_address.sin6.sin6_flowinfo	= address->sin6_flowinfo;
	memcpy(&(this->inet_address.sin6.sin6_addr), &(address->sin6_addr),
		sizeof(struct in6_addr));
	this->inet_address.sin6.sin6_scope_id	= address->sin6_scope_id;
	this->inet_address.sin6.sin6_family	= AF_INET6;
	if (IN6_IS_ADDR_MULTICAST(&(address->sin6_addr))) {
		this->is_multicast = true;
	} else {
		this->is_multicast = false;
	}
	this->is_inet6 = true;
}
#endif /* INET6 */


void
mcl_addr::incr_addr (INT32	incr)
{
	if (incr == 0) {
		return;
	}
#ifdef INET6
	if (this->is_ipv6_addr()) {
#ifdef WIN32
		struct in6_addr		in6;	// temp new IPv6 addr
		INT32			val;	// temp byte value

		in6 = this->inet_address.sin6.sin6_addr;
		// low byte in address is the 16th...
		// the following method assumes no overflow...
		val = ntohs(in6.s6_words[7]);
		val += incr;
		in6.s6_words[7] = htons(val);
		this->set_addr(&in6);
#else  /* Unix */
#ifdef SOLARIS		/* not defined on Solaris for user level code */
#define s6_addr32	_S6_un._S6_u32
#elif defined(FREEBSD)	/* not defined on FreeBSD for user level code */
#define s6_addr32	__u6_addr.__u6_addr32
#endif
		struct in6_addr		in6;	// temp new IPv6 addr
		INT32			val;	// temp byte value

		in6 = this->inet_address.sin6.sin6_addr;
		// low byte in address is the 16th...
		// the following method assumes no overflow...
		val = ntohl(in6.s6_addr32[3]);
		val += incr;
		in6.s6_addr32[3] = htonl(val);
		this->set_addr(&in6);
#endif /* OS */
	} else
#endif /* INET6 */
		this->set_addr(this->get_addr() + incr);
}


void
mcl_addr::incr_port (INT32	incr)
{
	this->set_port(this->get_port() + incr);
}


/****** GET FUNCTIONS *************************************************/

UINT16
mcl_addr::get_port() const
{
	/*
	 * Return the port.
	 * NB: the sin_port field is at the same place in sockaddr_in and
	 * sockaddr_in6 structures. No need to differentiate...
	 */
	return(ntohs(this->inet_address.sin.sin_port));
}


char*
mcl_addr::get_port_string () const
{
	static char	str[32];

	sprintf(str, "%d", this->get_port());
	return str;
}


UINT32
mcl_addr::get_addr() const
{
	/*
	 * Return the address.
	 */
#ifdef INET6
	if (this->is_ipv6_addr()) {
		PRINT_ERR((mcl_stderr,
		"mcl_addr::get_addr: ERROR, IPv4 function called for IPv6 addr.\n"))
		mcl_exit(-1);
	}
#endif /* INET6 */
	return(ntohl(this->inet_address.sin.sin_addr.s_addr));
}


void
mcl_addr::get_addr_struct(struct sockaddr_in *address) const
{
	/*
	 * Set address to the address structure.
	 */
#ifdef INET6
	if (this->is_ipv6_addr()) {
		PRINT_ERR((mcl_stderr,
		"mcl_addr::get_addr_struct: ERROR, IPv4 function called for IPv6 addr.\n"))
		mcl_exit(-1);
	}
#endif /* INET6 */
	*address = this->inet_address.sin;
}

#ifdef INET6
void
mcl_addr::get_addr_struct(struct sockaddr_in6 *address) const
{
	/*
	 * Set address to the address structure.
	 */
#ifdef INET6
	if (this->is_ipv6_addr() == false) {
		PRINT_ERR((mcl_stderr,
		"mcl_addr::get_addr_struct: ERROR, IPv6 function called for IPv4 addr.\n"))
		mcl_exit(-1);
	}
#endif /* INET6 */

	memcpy(address, &(this->inet_address.sin6),
		sizeof(struct sockaddr_in6));
}
#endif /* INET6 */


char*
mcl_addr::get_addr_string() const
{
	/* Warning: inet_ntoa is not re-entrant! */
#ifdef INET6
#ifdef WIN32
	if (this->is_ipv6_addr()) {
		static char		str[INET6_ADDRSTRLEN];
		DWORD			len = INET6_ADDRSTRLEN;
		struct sockaddr_in6	*sa6;
		INT32			port;

		/* we just want the address without port...
		 * temporarly set the port to 0 */
		sa6 = (struct sockaddr_in6*)&(this->inet_address.sin6);
		int port = sa6->sin6_port;
		sa6->sin6_port = htons(0);
		if (WSAAddressToString((struct sockaddr*) &(this->inet_address.sin6),
				sizeof(this->inet_address.sin6), NULL, str, &len) != 0) {
			PRINT_ERR((mcl_stderr,
				"mcl_addr::get_addr_string: ERROR, WSAStringToAddress failed: %i\n",
				 WSAGetLastError()))
		}
		/* set the port to its initial value */
		sa6->sin6_port = port;
		return(str);
	}
#else  /* UNIX */
	static char	str[INET6_ADDRSTRLEN];
	if (this->is_ipv6_addr()) {
		if (inet_ntop(AF_INET6, &(this->inet_address.sin6.sin6_addr),
				str, sizeof(str)) == NULL) {
			return((char*)"mcl_addr::get_addr_string: ERROR, inet_ntop failed\n");
		} else {
			return(str);
		}
	}
#endif /* OS */
	else
#endif /* INET6 */
		return(inet_ntoa(this->inet_address.sin.sin_addr));
}


#if 0
bool
mcl_addr::is_multicast_addr() const
{
	/*
	 * Check if the address is a multicast address.
	 */
	if ((ntohl(this->inet_address.sin_addr.s_addr) >> 24) >= 224) {
		return(true);
	} else {
		return(false);
	}
}
#endif


bool
mcl_addr::is_equal(const mcl_addr& address) const
{
	/*
	 * Check if the specified address is the "same".
	 */
#ifdef INET6
	if (address.is_ipv6_addr()) {
		if ((this->inet_address.sin6.sin6_family == address.inet_address.sin6.sin6_family)&&
		    (this->inet_address.sin6.sin6_port == address.inet_address.sin6.sin6_port) &&
		    memcmp(&(this->inet_address.sin6.sin6_addr),
			   &(address.inet_address.sin6.sin6_addr), sizeof(in6_addr)) == 0) {
			return(true);
		} else {
			return(false);
		}
	} else
#endif /* INET6 */
	{
		if ((this->inet_address.sin.sin_family == address.inet_address.sin.sin_family)&&
		    (this->inet_address.sin.sin_port == address.inet_address.sin.sin_port) &&
		    (this->inet_address.sin.sin_addr.s_addr == address.inet_address.sin.sin_addr.s_addr)) {
			return(true);
		} else {
			return(false);
		}
	}
}


bool
mcl_addr::addr_is_equal(const mcl_addr& address) const
{
	/*
	 * Check if the specified address is the "same".
	 * Do not consider the port, only the addr.
	 */
#ifdef INET6
	if (address.is_ipv6_addr()) {
		if ((this->inet_address.sin6.sin6_family == address.inet_address.sin6.sin6_family)&&
		    memcmp(&(this->inet_address.sin6.sin6_addr),
			   &(address.inet_address.sin6.sin6_addr), sizeof(in6_addr)) == 0) {
			return(true);
		} else {
			return(false);
		}
	} else
#endif /* INET6 */
	{
		if ((this->inet_address.sin.sin_family == address.inet_address.sin.sin_family)&&
		    (this->inet_address.sin.sin_addr.s_addr == address.inet_address.sin.sin_addr.s_addr)) {
			return(true);
		} else {
			return(false);
		}
	}
}

