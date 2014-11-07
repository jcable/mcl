/* $Id: mcl_osdep.h,v 1.16 2004/12/02 11:49:22 chneuman Exp $ */
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

/*
 * OS dependant code
 */
#ifndef _MCL_OSDEP_H_
#define _MCL_OSDEP_H_

/*
 * Since Linux is multi-platform, we define here two possible target
 * environments:
 *	- i386 for Intel/AMD hosts
 *	- ppc for PowerPC hosts
 * Change it as required (there's no automatic discovery)...
 * This is used in particular for bit-field order (see below).
 */
#ifdef LINUX
#define LINUX_I386		/* default */
/*#define LINUX_PPC*/		/* uncomment if needed */
#endif


/*
 * bit field order is compiler/OS dependant
 * With linux, see the above LINUX_XXX defines...
 */
#if defined(LINUX_I386) || defined(WIN32) || defined (FREEBSD)
#define _BIT_FIELDS_LTOH
#undef _BIT_FIELDS_HTOL

#elif defined(SOLARIS) || defined(LINUX_PPC) || defined(AIX) || defined(HPUX)

#undef _BIT_FIELDS_LTOH
#define _BIT_FIELDS_HTOL	/* SunSparc is HtoL */
#endif


/*
 * sighandler
 */
#if defined(LINUX) || defined (FREEBSD)
	/* On Linux systems, signal handlers must be of __sighandler_t type */
#define	sighandler_t	__sighandler_t

#elif defined(SOLARIS)

#define	sighandler_t	void (*)(int)
#endif /* OS */


/*
 * thread management functions and types
 */
#if defined(WIN32)
#define	mcl_mutex_t	CRITICAL_SECTION
#define mcl_thread_t	HANDLE
#else  /* UNIX */
#define	mcl_mutex_t	pthread_mutex_t
#define mcl_thread_t	pthread_t
#endif /* OS */


/*
 * mode_t for WIN32
 */
#ifdef WIN32
#define mode_t int
#endif

/*
 * socket and iovec structure macros for sendmsg/recvmsg syscalls
 */
#ifdef WIN32
#define MCL_SOCKET	SOCKET
#define random		rand
#define dup		_dup
#define MCL_IOVEC	WSABUF
#define MCL_IOV_BUFF(x)	x.buf
#define MCL_IOV_LEN(x)	x.len
#define MCL_IOV_BUFF_TYPE

#else  /* UNIX */

#define MCL_SOCKET	int
#define MCL_IOVEC	struct iovec
#define MCL_IOV_BUFF(x)	x.iov_base
#define MCL_IOV_LEN(x)	x.iov_len

#if defined(SOLARIS)
#define MCL_IOV_BUFF_TYPE	caddr_t
#elif defined(FREEBSD)
#define MCL_IOV_BUFF_TYPE	char*
#else
#define MCL_IOV_BUFF_TYPE	void*
#endif

#endif /* OS */


/**
 * Rounding functions
 */
extern INT32	double_to_closest_int (double value);
#ifdef WIN32
extern double	rint (double value);
#endif /* WIN32 */


/*
 * various stuff
 */
#ifdef WIN32
#define random		rand
#define dup		_dup
#else
#endif /* OS */


#ifdef SOLARIS
/**
 * inet_aton replacement for SOLARIS that doesn't include it
 * feel free to comment if already available on your system...
 */
extern int inet_aton(const char *cp, struct in_addr *inp);
#endif // SOLARIS


/*
 * MCL prototypes
 */
extern void	mcl_init_lock		(mcl_mutex_t *mutex);
extern void	mcl_lock		(mcl_mutex_t *mutex);
extern int	mcl_trylock		(mcl_mutex_t *mutex);
extern void	mcl_unlock		(mcl_mutex_t *mutex);
#ifdef ALC
extern void	mcl_thread_testcancel	(class mcl_cb *mclcb);
#endif

extern void	mcl_usleep		(unsigned long usec);
extern int	mcl_gettimeofday	(struct timeval*tv);
extern void	mcl_init_random		(void);
#ifdef WIN32
extern void	mcl_winsock_init	(void);
extern bool	mcl_is_valid_sock	(SOCKET sock);
#else  // UNIX
extern bool	mcl_is_valid_sock	(int sock);
#endif // OS


#ifdef NORM	/* { */

/*
 * The following defines are required by RMF code
 */

#if defined(LINUX) || defined(SOLARIS) || defined(FREEBSD)

//============================================================================
// NON-WIN32 SECTION
//============================================================================

#define mcl_STATIC_VOID_METHOD  static void *
#define mcl_VOID_METHOD         void *
#define mcl_SOCKET_TYPE         int
#define mcl_OPTVAL_TYPE         void *
#define mcl_TTL_TYPE            unsigned char


//============================================================================
// Mutex Section for non-Win32 Platforms -- Begin
//============================================================================

#define mcl_MUTEX_TYPE        pthread_mutex_t
#define mcl_MUTEX_CONST_INIT  PTHREAD_MUTEX_INITIALIZER

#define mcl_MUTEX_INIT     pthread_mutex_init
#define mcl_MUTEX_DESTROY  pthread_mutex_destroy
#define mcl_MUTEX_LOCK     pthread_mutex_lock
#define mcl_MUTEX_UNLOCK   pthread_mutex_unlock


//============================================================================
inline
INT32 mcl_MUTEX_STATIC_INIT(mcl_MUTEX_TYPE * m)
{
  return 0;
}


//============================================================================
// Mutex Section for non-Win32 Platforms -- End
//============================================================================


//============================================================================
// Condition Variable Section for non-Win32 Platforms -- Begin
//============================================================================

#define mcl_COND_VAR_TYPE       pthread_cond_t
#define mcl_COND_VAR_ATTR_TYPE  pthread_condattr_t

#define mcl_COND_VAR_INIT       pthread_cond_init
#define mcl_COND_VAR_DESTROY    pthread_cond_destroy
#define mcl_COND_VAR_WAIT       pthread_cond_wait
#define mcl_COND_VAR_BROADCAST  pthread_cond_broadcast


//============================================================================
inline
INT32 mcl_COND_VAR_TIMEDWAIT(mcl_COND_VAR_TYPE * c, mcl_MUTEX_TYPE * m,
                             INT32 msec)
{
  struct timeval   tv;
  struct timespec  ts;
  
#define GETTIMEOFDAY_NULL_ARG
#ifdef GETTIMEOFDAY_NULL_ARG
  gettimeofday(&tv, NULL);
#else
  gettimeofday(&tv);
#endif
  
  tv.tv_usec += (msec * 1000);
  tv.tv_sec  += (tv.tv_usec / 1000000);
  tv.tv_usec  = (tv.tv_usec % 1000000);
  
  ts.tv_sec  = tv.tv_sec;
  ts.tv_nsec = ((tv.tv_usec * 1000) + (msec * 1000000));
  
  return pthread_cond_timedwait(c, m, &ts);
}


//============================================================================
// Condition Variable Section for non-Win32 Platforms -- End
//============================================================================


//============================================================================
// Thread Section for non-Win32 platforms -- Begin
//============================================================================

typedef void *(*mcl_THREAD_FUNC_DEF)(void *);

#define mcl_THREAD_TYPE       pthread_t
#define mcl_THREAD_ATTR_TYPE  pthread_attr_t
#define mcl_THREAD_ATTR_PTR   mcl_THREAD_ATTR_TYPE *
#define mcl_THREAD_FUNC       mcl_THREAD_FUNC_DEF
#define mcl_THREAD_ARG        void *

#define mcl_CONST_CANCEL_ENABLE    PTHREAD_CANCEL_ENABLE
#define mcl_CONST_CANCEL_DEFERRED  PTHREAD_CANCEL_DEFERRED
#define mcl_CONST_CREATE_DETACHED  PTHREAD_CREATE_DETACHED

#define mcl_THREAD_ATTR_INIT            pthread_attr_init
#define mcl_THREAD_ATTR_SETDETACHSTATE  pthread_attr_setdetachstate
#define mcl_THREAD_ATTR_DESTROY         pthread_attr_destroy
#define mcl_THREAD_CREATE               pthread_create
#define mcl_THREAD_JOIN                 pthread_join
#define mcl_THREAD_DETACH               pthread_detach
#define mcl_THREAD_KILL                 pthread_cancel
#define mcl_THREAD_CANCEL_STATE         pthread_setcancelstate
#define mcl_THREAD_CANCEL_TYPE          pthread_setcanceltype
#define mcl_THREAD_TEST_CANCEL          pthread_testcancel
#define mcl_THREAD_YIELD                sched_yield


//============================================================================
inline
INT32 mcl_SIGNAL_BLOCK_MASK(INT32 signal)
{
  sigset_t  blockedsignals;
  
  sigemptyset(&blockedsignals);
  sigaddset(&blockedsignals, signal);
  return pthread_sigmask(SIG_BLOCK, &blockedsignals, NULL);
}


//============================================================================
// Thread Section for non-Win32 platforms -- End
//============================================================================


//============================================================================
inline
void mcl_INIT_WIN_SOCKETS()
{
  
}

#elif defined(WIN32)

TO DO...

#endif // OS_DEP

#endif /* } NORM */
#endif /* _MCL_OSDEP_H_ */
