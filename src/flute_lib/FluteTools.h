/* $Id: FluteTools.h,v 1.2 2005/05/12 16:03:41 moi Exp $ */
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

#ifndef FLUTE_TOOLS_H
#define FLUTE_TOOLS_H


class FluteTools {

public:

	FluteTools::FluteTools(class flute_cb * flutecb);
		
	FluteTools::~FluteTools();
	
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
	int extract_addr (char		*addr_string,
		      struct sockaddr	**addr,
		      int		port);

	
private:

	class flute_cb* flutecb;
};

#endif
