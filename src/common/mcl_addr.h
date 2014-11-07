/* $Id: mcl_addr.h,v 1.6 2004/06/02 13:37:33 roca Exp $ */
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

#ifndef MCL_ADDR_H
#define MCL_ADDR_H


/**
 * Class defining an IP address. 
 * Supports both IPv4 and IPv6.
 */
class mcl_addr {
 
public:
	/**
	 * Default constructor.
	 * By default creates a null AF_INET (i.e. IPv4) object.
	 */
	mcl_addr ();

	/**
	 * Default constructor.
	 * By default creates a null AF_INET (i.e. IPv4) object.
	 * @param is_ipv4	true if object is IPv4, false if object is IPv6
	 */
	mcl_addr (bool	is_ipv4);

	/**
	 * Copy constructor.
	 * @param address	The address object to copy.
	 */
	mcl_addr (const mcl_addr & address);

	/**
	 * The destructor.
	 */
	~mcl_addr ();

	/**
	 * The operator=.
	 * @param address	The address object to copy.
	 * @return		A reference to this object.
	 */
	mcl_addr & operator=(const mcl_addr & address);

	/**
	 * Manual reset of an address.
	 * This function resets all fields of the address.
	 * Usefull when the default constructor cannot be called.
	 * By default creates a null AF_INET struct.
	 */
	void	reset (void);


	/****** SET FUNCTIONS *************************************************/

	/**
	 * Set the port number.
	 * @param port		New port number in host byte order.
	 */
	void	set_port (UINT16 port);

	/**
	 * Set the port number to "any".
	 */
	void	set_any_port ();

	/**
	 * Set an IPv4 address.
	 * Restricted to IPv4 addresses.
	 * @param address	New address (INT32) in host byte order.
	 */
	void	set_addr (UINT32 address);

#ifdef INET6
	/**
	 * Set an IPv6 address.
	 * Restricted to IPv6 addresses.
	 * @param address       New address in host byte order.
	 */
	void	set_addr (struct in6_addr	*address);
#endif /* INET6 */

	/**
	 * Set the address to "any".
	 */
	void	set_any_addr ();

	/**
	 * Set the entire IPv4 or IPv6 address struct at one time.
	 * The sockaddr structure must be fully initialized, including
	 * the address family.
	 * @param address	New sockaddr structure.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_addr_struct (const struct sockaddr	*address);

	/**
	 * Set the entire IPv4 address struct at one time.
	 * @param address	New IPv4 sockaddr_in structure.
	 */
	void	set_addr_struct (const struct sockaddr_in	*address);

#ifdef INET6
	/**
	 * Set the entire IPv6 address struct at one time.
	 * @param address	New IPv6 sockaddr_in6 structure.
	 */
	void	set_addr_struct (const struct sockaddr_in6	*address);
#endif /* INET6 */

	/**
	 * Increment the IPv4 or IPv6 address value.
	 * @param	incr	value to add.
	 */
	void	incr_addr (INT32	incr);

	/**
	 * Increment the port value.
	 * @param	incr	value to add.
	 */
	void	incr_port (INT32	incr);


	/****** GET FUNCTIONS *************************************************/

	/**
	 * Return the address family (AF_INET or AF_INET6).
	 * @return		Address familly.
	 */
	INT32	get_addr_family () const;

	/**
	 * Return the address port number.
	 * @return		Address port number in host byte order.
	 */
	UINT16	get_port () const;

	/**
	 * Return the address port number as a string.
  	 * Warning: get_port_string is not re-entrant (static string)!
	 * @return		Address port number as a string.
	 */
	char*	get_port_string () const;

	/**
	 * Returns the sockaddr_in/sockaddr_in6 length.
	 * @reuturn		Length in bytes of the internal sockaddr_in
	 *			or sockaddr_in6 structure.
	 */
	INT32	get_addr_struct_len (void) const;

	/**
	 * Return the IPv4 address as a UINT32.
	 * @return		Address in host byte order.
	 */
	UINT32	get_addr () const;

	/**
	 * Get the entire IPv4 address at one time.
	 * @param address	Pointer to a sockaddr_in structure where
	 *			the address will be placed in network byte
	 *			order.
	 */
	void	get_addr_struct (struct sockaddr_in *address) const;

#ifdef INET6
	/**
	 * Get the entire IPv6 address at one time.
	 * @param address	Pointer to a sockaddr_in6 structure where
	 *			the address will be placed in network byte
	 *			order.
	 */
	void	get_addr_struct (struct sockaddr_in6 *address) const;
#endif /* INET6 */

	/**
	 * Return the internal sockaddr buffer address.
	 * Works for both IPv4 and IPv6 addresses.
	 * @return	address of internal sockaddr buffer.
	 */
	struct sockaddr*	get_internal_struct_addr (void);

	/**
	 * Return the address as a string in dot-decimal format.
  	 * Warning: inet_ntoa is not re-entrant, and get_addr_string isn't too!
	 * @return		Temporary character buffer containing
	 *			the string. This buffer is overwritten by
	 *			subsequent calls to this method.
	 */
	char*	get_addr_string () const;


	/****** TEST FUNCTIONS ************************************************/

	/**
	 * Checks if the address is a multicast address.
	 * @return		TRUE if this is a multicast address,
	 *			FALSE otherwise.
	 */
	bool	is_multicast_addr () const;

	/**
	 * Checks if the address is an IPv4 address.
	 * @return		TRUE if this is an IPv6 address,
	 *			FALSE otherwise.
	 */
	bool	is_ipv4_addr () const;

#ifdef INET6
	/**
	 * Checks if the address is an IPv6 address.
	 * @return		TRUE if this is an IPv6 address,
	 *			FALSE otherwise.
	 */
	bool	is_ipv6_addr () const;
#endif /* INET6 */

	/**
	 * Checks if the specified object is the "same" as this object.
	 * This function compares all fields of the address object.
	 * @param address	The address to compare against.
	 * @return		TRUE if the address is the "same" as
	 *			this object, FALSE otherwise.
	 */
	bool	is_equal (const mcl_addr & address) const;

	/**
	 * Checks if the IP address of the specified object is the "same"
	 * as this object.
	 * It does not consider the port info, only the IP address.
	 * @param address	The address to compare against.
	 * @return		TRUE if the address is the "same" as this
	 *			object, FALSE otherwise.
	 */
	bool	addr_is_equal (const mcl_addr & address) const;


	/****** PRIVATE *******************************************************/
private:

	/**
	 * The internet address structure in network byte order.
	 */
	union {
		struct sockaddr_in	sin;
#ifdef INET6
		struct sockaddr_in6	sin6;
#endif /* INET6 */
	} inet_address;

	bool		is_multicast;
	bool		is_inet6;	/* true if AF_INET6, false if AF_INET */
};


//----------------------------------------------------------------------------
// Inlines for all classes follow
//----------------------------------------------------------------------------

inline INT32
mcl_addr::get_addr_family () const
{
	return((this->is_inet6 == true) ? AF_INET6 : AF_INET);
}

inline bool
mcl_addr::is_multicast_addr() const
{
	return (this->is_multicast);
}

inline INT32
mcl_addr::get_addr_struct_len (void) const
{
#ifdef INET6
	if (this->is_inet6)
		return (sizeof(struct sockaddr_in6));
	else
#endif
		return (sizeof(struct sockaddr_in));
}

inline struct sockaddr*
mcl_addr::get_internal_struct_addr ()
{
	return ((struct sockaddr*) &(this->inet_address));
}

inline bool
mcl_addr::is_ipv4_addr () const
{
	return (!(this->is_inet6));
}

#ifdef INET6
inline bool
mcl_addr::is_ipv6_addr () const
{
	return (this->is_inet6);
}
#endif /* INET6 */

#endif	// MCL_ADDR_H

