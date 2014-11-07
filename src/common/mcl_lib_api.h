/* $Id: mcl_lib_api.h,v 1.14 2005/03/18 12:06:20 roca Exp $ */
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

#ifndef MCL_LIB_API_H
#define MCL_LIB_API_H

#include "mcl_types.h"	/* define portable data types */


/* 
 * MCL API definition.
 * This is the ONLY header file that must be included by all external
 * applications!
 *
 * This common part of the MCL API includes a reliable multicast specific
 * part, depending on whether ALC or NORM is used.
 * This RM-specific part defines the various control parameters that can
 * be set with mcl_ctl(). You should have a look at it if you build an
 * application on top of MCL.
 */


/****** ALC or NORM Specific Part *********************************************/

/* 
 * library options set with the mcl_ctl() function are defined in these
 * include files.
 */
#ifdef ALC
#include "../alc/mcl_lib_api_alc.h"
#elif defined NORM
#include "../norm/mcl_lib_api_norm.h"
#else
Error, either ALC or NORM should be defined when including this file according
to the reliable multicast protocol you want to use...
#endif


/****** Common Part ***********************************************************/

/**
 * Open an MCL-ALC or MCL-NORM session.
 * Wether it uses ALC or NORM as its reliable multicast protocol depends
 * on the compilation parameters. The same application cannot concurrently
 * have ALC and NORM sessions, they are mutually exclusive (this is a
 * limitation of the current implementation!).
 *
 * @param  mode	defines the kind of session ("r", "w")
 * @return	returns to appli a unique identifier if ok, or < 0 in case of error
 */
extern INT32	mcl_open	(const char *mode);

/**
 * Close the MCL session.
 *
 * @param  id	MCL session identifier
 * @return	returns 0 if ok, or < 0 in case of error
 */
extern int	mcl_close	(INT32 id);

extern int	mcl_abort	(INT32 id);

/**
 * Get or set control parameters for the session.
 * Follows the Unix ioctl() syscall interface.
 * Warning: usually the tx and rx contexts are still not fully initialized!
 * This is only done during the first call to mcl_send or mcl_recv...
 *
 * @param id	MCL session identifier
 * @param optname	the option to set/get/change
 * @param optvalue	the associated value to set or to read. It is
 * 			either a simple type (e.g. INT32) or a structure
 * 			(e.g. struct sockaddr). This value can be either
 * 			set in MCL or read from MCL's internals and
 * 			returned to the appli.
 * @param optlen	the length in bytes of the associated value field
 * @return		returns 0 if ok, or < 0 in case of error
 */
extern int	mcl_ctl		(INT32 id, INT32 optname, void *optvalue,
				 INT32 optlen);

/**
 * mcl_send data sending function.
 *
 * @param id	MCL session identifier
 * @param data	data block; in MCL_OPT_REUSE_APPLI_TX_BUFFER mode, the
 * 		data pointer must be the result of a standard
 * 		malloc/calloc/realloc function call.
 * @param len	data block length in bytes
 * @return	number of bytes sent or -1 if error
 */
extern INT32	mcl_send	(INT32 id, const void *data, INT32 len);

/**
 * mcl_sendto data sending function.
 *
 * @param id	MCL session identifier
 * @param data	data block; in MCL_OPT_REUSE_APPLI_TX_BUFFER mode, the
 * 		data pointer must be the result of a standard
 * 		malloc/calloc/realloc function call.
 * @param len	data block length in bytes
 * @param saddr	destination sockaddr structure corresponding to an IPv4
 *		or IPv6 address
 * @param saddr_len	sockaddr structure actual length (the sockaddr 
		struct is generic!)
 * @return	number of bytes sent or -1 if error
 */
extern INT32	mcl_sendto	(INT32 id, const void *data, INT32 len,
				 const struct sockaddr *saddr, INT32 saddr_len);

/**
 * mcl_recv data reception function.
 *
 * @param id	MCL session identifier
 * @param buf	pointer to data buffer
 * @param len	data buffer length in bytes
 * @return	number of bytes received or -1 if error
 */
extern INT32	mcl_recv	(INT32 id, void *buf, INT32 len);

/**
 * mcl_recvfrom data reception function.
 *
 * @param id	MCL session identifier
 * @param data	pointer to data buffer
 * @param len	data buffer length in bytes
 * @param saddr	source address from which data was received
 * 			(so far only AF_INET is supported)
 * @param saddr_len	pointer to address length (sockaddr struct is generic!).
 * 			This argument is modified by mcl_recvfrom to contain
 * 			the actual length of the address at function return.
 * @return	number of bytes received or -1 if error
 */
extern INT32	mcl_recvfrom	(INT32 id, void * buf, INT32 len,
				 struct sockaddr *saddr, INT32 *saddr_len);

#ifdef ALC /* { */
/**
 * Flags that can be used by the mcl_sndmsg/mcl_recvmsg functions.
 */
enum mcl_msgflag {
	MCL_MSG_DEFAULT = 0,	/* This flag is the default behavior. */
				/* At a receiver, copy data and remove it */
				/* from the received and decoded object list, */
				/* at a sender, add data to the object list. */
#if 0	/* not yet */
	MCL_MSG_PEEK,		/* This flag causes the receive operation */
				/* to return data from the beginning of the */
				/* receive queue without removing that data */
				/* from the queue. Thus, the following */
				/* receive call will return the same data. */
#endif
	MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT
				/* This flag causes the receive application */
				/* to return the amount of data available for */
				/* a future read, as well as the associated
				 * TOI. Usefull to have an idea of the buffer */
				/* size required for a future read, and/or */
				/* determining which object is available when */
				/* used with FLUTE. This call requires to */
				/* use a mcl_msghdr struct, with a null iov */
				/* array (msg_iov = NULL && msg_iovlen=0). */
				/* When mcl_recvmsg is called with this flag, */
				/* the subsequent mcl_recv/recv_flute/recvmsg */
				/* call MUST return the same object. */
				/* This flag is not valid at a sender. */
};


/**
 * Structure for scatter/gather I/O, and used by the mcl_sndmsg/mcl_recvmsg
 * functions.
 */
struct mcl_iovec {
	INT32	iov_type;	/* type of storage (file or buffer) */
	void	*iov_base;	/* pointer to buffer storage area in */
				/* MCL_IOV_TYPE_BUFFER mode */
	char	*iov_filename;	/* file name to be used as storage area in */
				/* MCL_IOV_TYPE_FILE mode */
	INT64	iov_offset;	/* file offset from begining of file, in */
				/* MCL_IOV_TYPE_FILE type */
	INT64	iov_len;	/* length to consider in buffer or file */
};

#define MCL_IOV_TYPE_BUFFER	0
#define MCL_IOV_TYPE_FILE	1


/**
 * Message control structure used by mcl_sndmsg/mcl_recvmsg functions.
 */
struct mcl_msghdr {
	void		*msg_name;	/* optional address. When present, */
					/* it must point to a sockaddr struct */
	INT32		msg_namelen;	/* size of address. When present, */
					/* it must be equal to */
					/*  sizeof(struct sockaddr_in[6]) */
	struct mcl_iovec *msg_iov;	/* scatter/gather array */
	INT32		msg_iovlen;	/* number of elements in msg_iov */
	//void		*msg_control;	/* ancillary data, see below */
	//INT32		msg_controllen;	/* ancillary data buffer len */
	//INT32		msg_flags;	/* flags on received message */
	UINT64		toi;		/* transport object ID. */
					/* Updated by mcl_recvmsg() */
};


/**
 * mcl_sendmsg extended data transmission function.
 * This function, similar in spirit to sendmsg, enables a full control of
 * what data is submitted by the application at the price of a higher
 * complexity. For instance, data from a file, at an appropriate offset,
 * can be submitted directly, or gathered from several buffers/files (or
 * any mix of them) to the same ALC object, etc.
 * WARNING: in current implementation, a single iovec entry is assumed
 *	    (i.e. does not support data gathering)!
 *
 * @param id	MCL session identifier
 * @param msg	pointer to the mcl_msghdr structure.
 * @param flags	flags to control some specific features.
 * @return	number of bytes sent, or -1 if an error occured.
 */
extern INT64	mcl_sendmsg	(INT32 id, struct mcl_msghdr *msg,
				 mcl_msgflag flags);


/**
 * mcl_recvmsg extended data reception function.
 * This function, similar in spirit to recvmsg, enables a full control of
 * what data is returned to the application at the price of a higher
 * complexity. For instance, data can be stored directly to a file, at
 * an apprppriate offset, or scattered into several different buffers/files
 * (or any mix of them), etc.
 *
 * @param id	MCL session identifier
 * @param msg	pointer to the mcl_msghdr structure.
 * @param flags	flags to control some specific features.
 * @return	number of bytes received (or available if MCL_MSG_PEEK or
 * 		MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT are used), or -1 if
 * 		an error occured.
 */
extern INT64	mcl_recvmsg	(INT32 id, struct mcl_msghdr *msg,
				 mcl_msgflag flags);
#endif /* } ALC */


#ifdef ALC
/**
 * mcl_wait_event waits (i.e. blocks) until a given event takes
 * place and then returns.
 * This function is only defined with MCL-ALC sessions.
 *
 * @param id	MCL session identifier
 * @param event	event type
 * @return	0 if ok, -1 if error
 */
extern INT32	mcl_wait_event	(INT32 id, INT32 event);


/**
 * mcl_select waits until an event occured in one of its file
 * descriptors or until it timeout's.
 * This function currently has many limitations:
 *  - it only waits until on readfs descriptors, other file descriptor
 *    pointers must be NULL
 *  - it only considers a single readfds, waiting on multiple MCL sessions
 *    is not supported
 *  - only MCL session descriptors can be specified in the file descriptor
 *    set. It is not possible to mix non-MCL and MCL descriptors
 *  - it is only available on MCL-ALC sessions.
 * The behavior and parameters are otherwise similar to that of the
 * socket select() system call.
 * @param n	max MCL session identifier plus 1
 * @param readfs	table of MCL session identifier (only one is
 *			currently possible)
 * @param writefs	not supported, must be NULL
 * @param exceptfs	not supported, must be NULL
 * @param timeout	pointer to a timeval structure, indicating how
 *			long select can wait. If NULL, this function
 *			returns immediately
 * @return	On success, it return the number of descriptors contained
 *		in the descriptor sets, which may be zero if the timeout
 *		expires before anything interesting happens.
 *		On error -1 is returned and errno is set appropriately;
 *		the sets and timeout become undefined, so do not rely on
 *		their contents after an error.
 */
extern INT32	mcl_select	(INT32 n, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout);


/*
 * for mcl_wait_event...
 */
#define	MCL_WAIT_EVENT_END_TX	0
#define	MCL_WAIT_EVENT_END_RX	1
#define	MCL_WAIT_EVENT_CLOSED	2

#endif /* ALC */


#endif /* MCL_LIB_API_H */
