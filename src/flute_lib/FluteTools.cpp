/* $Id: FluteTools.cpp,v 1.3 2005/05/23 11:11:18 roca Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
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

#include "flute_includes.h"

FluteTools::FluteTools(class flute_cb *flutecb){
	this->flutecb = flutecb;
}


FluteTools::~FluteTools(){
}

/**
 * Extract an IPv4 or IPv6 address from an address string or host name.
 * Copies the address in the sockaddr structure.
 * @param addr_string	IPv4/IPv6 address string or host name.
 * @param addr		address of the pointer to sockaddr where to store
 * 			the result. This function allocates the appropriate
 * 			sockaddr_in/sockaddr_in6 structure.
 * @param port		port number in HOST byte order to store in sockaddr.
 * @return		0 if OK, < 0 in case of error.
 */
int FluteTools::extract_addr (char		*addr_string,
			      struct sockaddr	**addr,
			      int		port)
{
// choose one resolver method according to what is available...
#if defined(LINUX)
#define HAS_GETHOSTBYNAME2	// GNU extension, ok on linux
#elif defined(SOLARIS) || defined(FREEBSD)
#define HAS_GETIPNODEBYNAME	// POSIX  1003.1-2001 version
#elif defined(WIN32)
#define HAS_NOTHING	
#endif
	
	
#ifdef HAS_GETHOSTBYNAME2
	struct hostent	*hp;

	/* search for IPv4/IPv6 hostname or address first */
	hp = gethostbyname2(addr_string, AF_INET);
	if (hp != NULL) {
		struct sockaddr_in	*sa;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		memcpy(&(sa->sin_addr.s_addr),
			hp->h_addr_list[0],
			sizeof(struct in_addr));
		return 0;
	}
	hp = gethostbyname2(addr_string, AF_INET6);
	if (hp != NULL) {
		struct sockaddr_in6	*sa6;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		memcpy(&(sa6->sin6_addr), hp->h_addr_list[0],
			sizeof(struct in6_addr));
		return 0;
	}
	/* everything failed */
	PRINT(("Flute: ERROR, unknown host ""%s""\n", addr_string))
	return -1;
#endif // HAS_GETHOSTBYNAME2
#ifdef HAS_GETIPNODEBYNAME
	struct hostent	*hp;
	int		err;

	/* search for IPv4 hostname or address first */
	hp = getipnodebyname(addr_string, AF_INET, AI_DEFAULT, &err);
	//PRINT(("search for IPv4: returned x%x, err=%d\n", hp, err))
	if (hp != NULL) {
		struct sockaddr_in	*sa;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		memcpy(&(sa->sin_addr.s_addr),
			hp->h_addr_list[0],
			sizeof(struct in_addr));
		return 0;
	}
	/* search for IPv6 hostname or address then */
	hp = getipnodebyname(addr_string, AF_INET6, AI_DEFAULT, &err);
	//PRINT(("search for IPv6: returned x%x, err=%d\n", hp, err))
	if (hp != NULL) {
		struct sockaddr_in6	*sa6;
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(port);
		memcpy(&(sa6->sin6_addr), hp->h_addr_list[0],
			sizeof(struct in6_addr));
		return 0;
	}
	/* everything failed */
	PRINT(("Flute: ERROR, unknown host ""%s""\n", addr_string))
	return -1;
#endif // HAS_GETIPNODEBYNAME
#ifdef HAS_NOTHING

	if (flutecb->ip_version == 4)
	{
		struct sockaddr_in	*sa;
	
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa = (struct sockaddr_in*)*addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(port);
		/*
		 * addr must be in host byte order, but
		 * gethostbyname/inet_addr return network order
		 */
		if (isalpha((int)addr_string[0])) {
			if (gethostbyname(addr_string) == NULL) {
				PRINT(("Flute: ERROR, unknown host ""%s""\n", addr_string))
				PRINT(("Flute: did you specify the right IP version?\n"))
				free(*addr);
				*addr = NULL;
				return -1;
			}
			sa->sin_addr.s_addr = *(unsigned long *)((gethostbyname(addr_string))->h_addr);
		} else {
			sa->sin_addr.s_addr = inet_addr(addr_string);
		}
	}
	else if (flutecb->ip_version == 6)
	{
		struct sockaddr_in6	*sa6;
		int len = sizeof(struct sockaddr_in6);
		/* addr must be in host byte order, but
		 * hostent is in network order! */
		*addr = (struct sockaddr*)
				calloc(1, sizeof(struct sockaddr_in6));
		if (*addr == NULL) {
			EXIT(("Flute: ERROR, no memory\n"))
		}
		sa6 = (struct sockaddr_in6*)*addr;
		sa6->sin6_family = AF_INET6;
		if (WSAStringToAddress(addr_string, AF_INET6, NULL, (struct sockaddr*) sa6, &len) != 0)
		{
			EXIT(("Flute: ERROR, WSAStringToAddress failed: %i\n",  WSAGetLastError()))
		}
		sa6->sin6_port = htons(port);
	}

	return 0;
#endif // HAS_NOTHING
}
		

