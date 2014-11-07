/* $Id: mcl_socket.cpp,v 1.22 2005/05/24 15:43:22 roca Exp $ */
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

#include "mcl_includes.h"


/**
 * Constructor.
 */
mcl_socket::mcl_socket ()
{
	this->addrinfo = NULL;
	this->reset();
}


/**
 * Destructor.
 */
mcl_socket::~mcl_socket ()
{
	this->reset();
}


/**
 * Reset the class.
 * => See header file for more informations.
 */
void
mcl_socket::reset ()
{

	this->id = -1;
#ifdef WIN32
	this->ses_sock = this->priv_sock = INVALID_SOCKET;
#else  /* UNIX */
	this->ses_sock = this->priv_sock = 0;
#endif
	if (this->addrinfo) {
		freeaddrinfo(this->addrinfo);
		this->addrinfo = NULL;
	}
	this->tx_state = SOCK_TX_INVALID;
	this->rx_state = SOCK_RX_INVALID;
}


/**
 * Initialize as a sender.
 * => See header file for more informations.
 */
mcl_error_status
mcl_socket::init_as_sender	(mcl_cb		*const mclcb,
				 INT32		lay_id,
				 mcl_addr	*ses_addr,
				 mcl_addr	*ifaddr,
				 char		*ifname)
{
	INT32			err;		/* error code */
	INT32			mode;		/* sender and/or receiver? */
	struct addrinfo		hints;

	// split TRACELVL into two part to avoid non-reentrant problems
	// of get_addr_string()
	TRACELVL(5, (mcl_stdout,
		"-> mcl_socket::init_as_sender: lay_id=%d ses_addr=%s/%s",
		lay_id, ses_addr->get_addr_string(),
		ses_addr->get_port_string()))
	TRACELVL(5, (mcl_stdout, " ifaddr=%d ifname=%s\n",
		(ifaddr ? ifaddr->get_addr_string() : "null"),
		(ifname ? ifname : "null")))
	ASSERT(mclcb->is_a_sender());
	mode = mclcb->ucast_mcast_mode;
	this->id = lay_id;
	this->addr = *ses_addr;
	/*
	 * This is a sender, in either MODE_UNI_TX or MODE_MCAST_TX.
	 */
	this->priv_sock = socket(this->addr.get_addr_family(), SOCK_DGRAM, 0);
	if (mcl_is_valid_sock(this->priv_sock) == false) {
		PRINT_ERR((mcl_stderr, "mcl_socket::init_as_sender: ERROR, socket() failed"))
		goto bad;
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = this->addr.get_addr_family();
	hints.ai_socktype = SOCK_DGRAM;
	err = getaddrinfo(this->addr.get_addr_string(),
			  this->addr.get_port_string(),
			  &hints, &(this->addrinfo));
	if (err != 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_sender: ERROR, %s\n",
			gai_strerror(err)))
		goto bad;
	}
	/*
	 * multicast specific init
	 */
	if (mode & MODE_MCAST_TX) {
		if (this->addr.is_ipv4_addr()) {
#ifdef WIN32
			/* Windows requires to bind to 0/INADDR_ANY */
			SOCKADDR_IN source_sin;
			source_sin.sin_family = AF_INET;
			source_sin.sin_port = htons(0);  
			source_sin.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(this->priv_sock,
				 (struct sockaddr FAR *)&source_sin,
				 sizeof (source_sin)) == SOCKET_ERROR) {
				perror("mcl_socket::init_as_sender: ERROR, bind failed");
				goto bad;
			}
#else  /* UNIX */
			/* do not bind on this socket to get a locally unique
			 * port number */
#endif /* OS */
		} else {
			/* to bind or not to bind? */
		}
		/* specify multicast interface */
		if (//ifname != NULL &&
		    mcast_set_if(this->priv_sock,
				 this->addr.get_addr_family(),
				 ifaddr, ifname,
				 mclcb->get_verbosity()) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_sender: ERROR, mcast_set_if failed for interface name \"%s\"\n", ifname))
			goto bad;
		}
		/* specify ttl */
		if (mcast_set_ttl(this->priv_sock,
				  this->addr.get_addr_family(),
				  mclcb->ttl) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_sender: ERROR, mcast_set_ttl failed for ttl=%d\n", mclcb->ttl))
			goto bad;
		}
#ifndef WIN32
		/* turn on the loop back of multicast packets */
		if (mcast_set_loop(this->priv_sock,
				   this->addr.get_addr_family(), 1) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_sender: ERROR, mcast_set_loop failed\n"))
			goto bad;
		}
#endif /* NOT WIN32 */
	}
	ASSERT(mcl_is_valid_sock(this->priv_sock));
	TRACELVL(4, (mcl_stdout,
		"mcl_socket::init_as_sender: layer %d <=> priv_sock=%d: %s/%d\n",
		lay_id, (INT32)this->priv_sock, this->addr.get_addr_string(),
		this->addr.get_port()))
	/* done */
	this->tx_state = SOCK_TX_INITIALIZED;
	TRACELVL(5, (mcl_stdout, "<- mcl_socket::init_as_sender:\n"))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr, "mcl_socket::init_as_sender: ERROR\n"))
	return MCL_ERROR;
}


/**
 * Initialize as a receiver.
 * => See header file for more informations.
 */
mcl_error_status
mcl_socket::init_as_receiver	(mcl_cb		*mclcb,
				 INT32		lay_id,
				 mcl_addr	*ses_addr,
#ifdef SSM
				 mcl_addr	*src_addr,
#endif
				 mcl_addr	*ifaddr,
				 char		*ifname)
{
	INT32			err;		/* error code */
	INT32			mode;		/* sender and/or receiver? */
	struct addrinfo		hints;
#ifdef WIN32
	u_long			set_non_blocking = 1;	/* non blocking read */
							/* NB: must be u_long */
#endif /* OS_DEP */
	UINT32			reuse_addr;	/* for setsockopt */
#ifdef SOLARIS
	INT32			val;		/* temporary */
#endif

	// split TRACELVL into two part to avoid non-reentrant problems
	// of get_addr_string()
	TRACELVL(5, (mcl_stdout,
		"-> mcl_socket::init_as_receiver: lay_id=%d ses_addr=%s/%s",
		lay_id, ses_addr->get_port_string(),
		ses_addr->get_addr_string()))
	TRACELVL(5, (mcl_stdout, " ifaddr=%s ifname=%s\n",
		(ifaddr ? ifaddr->get_addr_string() : "null"),
		(ifname ? ifname : "null")))
	ASSERT(mclcb->is_a_receiver());
	mode = mclcb->ucast_mcast_mode;
	this->id = lay_id;
	this->addr = *ses_addr;
	/*
	 * This is a receiver, in either MODE_UNI_RX or MODE_MCAST_RX
	 * The session socket is always used for mcast rx...
	 */
	this->ses_sock = socket(this->addr.get_addr_family(), SOCK_DGRAM, 0);
	if (mcl_is_valid_sock(this->ses_sock) == false) {
		PRINT_ERR((mcl_stderr, "mcl_socket::init_as_receiver: ERROR, socket failed"))
		goto bad;
	}
	/*
	 * set non-blocking mode for future read on the session sock.
	 * we noticed that it largely accelerates packet reception
	 * (see mcl_network.cpp:mcl_recv_pkt()).
	 * Using the default blocking mode requires to always call
	 * select() before trying to read packets. In non-blocking
	 * mode we can read() at any time.
	 */
#ifdef WIN32
	if (ioctlsocket(this->ses_sock, FIONBIO, (&set_non_blocking)) == SOCKET_ERROR ) {
		perror("mcl_socket::init_as_receiver: ERROR, fcntl failed");
		mcl_exit(1);
	}
#else
	if (fcntl(this->ses_sock, F_SETFL, O_NONBLOCK) < 0) {
		perror("mcl_socket::init_as_receiver: ERROR, fcntl failed");
		mcl_exit(1);
	}
#endif
#ifdef SOLARIS
	/*
	 * Increase the default socket size.
	 * must be large enough to absort packet bursts on each layer!
	 * Solaris 2.8:
	 * 	SunOS sets the maximum  buffer  size
	 *	for both  UDP and TCP to 256 Kbytes.
	 */
#define MAX_RX_SOCK_SIZE	(256*1024)
	val = MCL_RX_SOCK_SIZE;
	if (setsockopt(this->ses_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&val, sizeof(val)) < 0) {
		perror("mcl_socket::init_as_receiver: ERROR, SO_RCVBUF failed");
		/* non critical error, so ignore */
	}
#else
	/* On Linux use the default value.
	 * System-level values can be changed manually as explained below.
	 * 
	 * (man -s7 socket): SO_RCVBUF
	 * Sets or gets the maximum socket receive  buffer  in  bytes.  The
	 * default  value is set by the rmem_default sysctl and the maximum
	 * allowed value is set by the rmem_max sysctl.
	 *
	 * So don't set anything, this can be done automatically with an
	 * appropriate sysctl.
	 * By default, on a 2.6.9 kernel:
	 *	$ sysctl net.core.rmem_default
	 *	net.core.rmem_default = 110592
	 *	$ sysctl net.core.rmem_max
	 *	net.core.rmem_max = 131071
	 */
#endif
	/* for the future select() */
	FD_SET((u_int)this->ses_sock, &(mclcb->rxlvl.fds));
	mclcb->rxlvl.nfds = max(mclcb->rxlvl.nfds,
				(int)this->ses_sock + 1);
	mclcb->rxlvl.n_fd++;

	/* allow for reuse of the couple addr/port */
	reuse_addr = 1;
	if (mclcb->addr.is_multicast_addr() &&
	    setsockopt(this->ses_sock, SOL_SOCKET, SO_REUSEADDR,
		       (char *)&reuse_addr, sizeof(reuse_addr)) < 0) {
		perror("mcl_socket::init_as_receiver: ERROR, REUSEADDR failed");
		mcl_exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = this->addr.get_addr_family();
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	err = getaddrinfo(this->addr.get_addr_string(), this->addr.get_port_string(),
			  &hints, &(this->addrinfo));
	if (err != 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_receiver: ERROR, %s\n",
			gai_strerror(err)))
		goto bad;
	}

	/* specify INADDR_ANY */
	if (this->addrinfo->ai_family == AF_INET) {
		((sockaddr_in *) this->addrinfo->ai_addr)->sin_addr.s_addr = 
						htonl(INADDR_ANY);
	}
#ifdef INET6
	else if (this->addrinfo->ai_family == AF_INET6) {
		((sockaddr_in6 *) this->addrinfo->ai_addr)->sin6_addr = 
						in6addr_any;
	}
#endif

	/* now bind the session port number (with INADDR_ANY addr) */
	if (bind(this->ses_sock,  this->addrinfo->ai_addr,
			this->addrinfo->ai_addrlen) < 0) {
		perror("mcl_socket::init_as_receiver: bind");
		PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_receiver: ERROR, bind failed\n"))
		goto bad;
	}

	if (mode & MODE_MCAST_RX) {
		/* specify multicast interface */
		if (mcast_set_if(this->ses_sock,
				 this->addr.get_addr_family(),
				 ifaddr, ifname,
				 mclcb->get_verbosity()) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_receiver: ERROR, mcast_set_if failed for ifname %s\n", ifname))
			goto bad;
		}
#ifndef WIN32
		/* turn off the loop back of multicast packets */
		/* XXX: is it really required ? */
		if (mcast_set_loop(this->ses_sock,
				   this->addr.get_addr_family(), 0) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::init_as_receiver: ERROR, mcast_set_loop failed\n"))
			goto bad;
		}
#endif
		/* ASM or SSM multicast join */
#ifdef SSM
		if (mclcb->rx.ssm) {
			if (mclcb->rx.check_src_addr == false) {
				/* in SSM mode but source addr not specified */
				PRINT_ERR((mcl_stderr,
				"mcl_socket::init_as_receiver: ERROR, SSM mode set but source address not specified\n"))
				goto bad;
			}
			if (ssm_mcast_join (this->ses_sock, 
					this->addr.get_internal_struct_addr(),
					mclcb->rx.src_addr.get_internal_struct_addr(),
					ifaddr, ifname,
					mclcb->get_verbosity()) < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_socket::init_as_receiver: ERROR, mcast_join failed\n"))
				goto bad;
			}
		} else
#endif /* SSM */
		{
			if (mcast_join (this->ses_sock, 
					this->addr.get_internal_struct_addr(),
					ifaddr, ifname) < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_socket::init_as_receiver: ERROR, mcast_join failed\n"))
				goto bad;
			}
		}
	}

	ASSERT(mcl_is_valid_sock(this->ses_sock));
	TRACELVL(4, (mcl_stdout,
	"mcl_socket::init_as_receiver: layer %d <=> sess_sock=%d: %s/%d\n",
		lay_id, (int)this->ses_sock, this->addr.get_addr_string(),
		this->addr.get_port()))
	/* done */
	this->rx_state = SOCK_RX_INITIALIZED;
	TRACELVL(5, (mcl_stdout, "<- mcl_socket::init_as_receiver:\n"))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr, "mcl_socket::init_as_receiver: ERROR\n"))
	return MCL_ERROR;
}


/**
 * Close the sockets.
 * => See header file for more informations.
 */
mcl_error_status
mcl_socket::close_sockets (mcl_cb	*const mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_close_sockets:\n"))
	if (mclcb->ucast_mcast_mode & MODE_MCAST_RX) {
		/* leave mcast group first */
#ifdef SSM
		if (mclcb->rx.ssm) {
			if (ssm_mcast_leave (this->ses_sock, 
					this->addr.get_internal_struct_addr(),
					mclcb->rx.src_addr.get_internal_struct_addr(),
					mclcb->mcast_if_addr,
					mclcb->mcast_if_name,
					mclcb->get_verbosity()) < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_socket::close_sockets: ERROR, ssm_mcast_leave failed\n"))
				goto bad;
			}
		} else 
#endif
		if (mcast_leave (this->ses_sock, 
				this->addr.get_internal_struct_addr(),
				mclcb->mcast_if_addr,
				mclcb->mcast_if_name) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_socket::close_sockets: ERROR, mcast_leave failed\n"))
			goto bad;
		}
	}
	if (this->can_rx()) {
		/* remove from select() */
		FD_CLR(this->ses_sock, &(mclcb->rxlvl.fds));
		if (mclcb->rxlvl.nfds == (int)this->ses_sock + 1) {
			/* TODO: WIN32 compliant? */
			mclcb->rxlvl.nfds--;
		}
		mclcb->rxlvl.n_fd--;
	}

	if (mcl_is_valid_sock(this->priv_sock)) {
#ifdef WIN32
		closesocket(this->priv_sock);
		this->priv_sock = INVALID_SOCKET;
#else /* UNIX */
		close(this->priv_sock);
		this->priv_sock = 0;
#endif
		this->tx_state = SOCK_TX_INVALID;
	}
	if (mcl_is_valid_sock(this->ses_sock)) {
#ifdef WIN32
		closesocket(this->ses_sock);
		this->ses_sock = INVALID_SOCKET;
#else /* UNIX */
		close(this->ses_sock);
		this->ses_sock = 0;
#endif
		this->rx_state = SOCK_RX_INVALID;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_close_sockets:\n"))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr, "mcl_socket::close_sockets: ERROR\n"))
	return MCL_ERROR;
}

