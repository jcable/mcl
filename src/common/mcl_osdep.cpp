/* $Id: mcl_osdep.cpp,v 1.10 2004/12/10 13:21:12 chneuman Exp $ */
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

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
extern void	mcl_exit (int n);
#endif // ALC|NORM



/**
 * initialize a mutex lock
 * @param mutex mutex to initialize
 */
void
mcl_init_lock (mcl_mutex_t	*mutex)
{
#if defined(WIN32)
	InitializeCriticalSection(mutex);
#else
	{
	pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;
	memcpy(mutex, &mutex_init, sizeof(pthread_mutex_t));
	}
#endif
}


//#ifdef ALC
#ifdef NEVERDEF
/**
 * get the MCL global lock
 * @param mclcb 
 */
void
mcl_global_lock (mcl_cb	*mclcb)
{
	mcl_lock(&mcl_mutex_lock);
}
#endif // ALC


/**
 * get this lock
 * @param mutex mutex lock
 */
void
mcl_lock (mcl_mutex_t *mutex)
{
#ifdef WIN32
	EnterCriticalSection(mutex);
#else
	pthread_mutex_lock(mutex);
#endif /* OS_DEP */
}


/*
 * try to get a lock
 * @param mclcb
 * @return return EBUSY if not possible, 0 if ok
 */
int
mcl_trylock (mcl_mutex_t *mutex)
{
#ifdef WIN32
	return 1;		/* non 0 to simulate error */
#else
	return pthread_mutex_trylock(mutex);
#endif /* OS_DEP */
}


/**
 * release a lock
 * @param mutex mutex lock
 */
void
mcl_unlock (mcl_mutex_t *mutex)
{
#ifdef WIN32
	LeaveCriticalSection(mutex);
#else
	pthread_mutex_unlock(mutex);
#endif /* OS_DEP */
}


#ifdef ALC
void
mcl_thread_testcancel (mcl_cb	*mclcb)
{
#ifdef WIN32
	if(mclcb->test_cancel == true) {
		ExitThread(0);
	}
#else /* UNIX */
	pthread_testcancel();
#endif
}
#endif /* ALC */


/**
 * portable micro sleep
 * replaces: void usleep(unsigned long usec)
 * while conserving the same prototype
 * @ param usec time to sleep in micro-seconds
 */
void
mcl_usleep (unsigned long usec)
{
#ifdef WIN32
	Sleep(max(usec,1000)/1000);
#elif defined(LINUX)
	usleep(usec);
#else
	struct timeval tv;

	tv.tv_sec = ceil(usec / 1000000);
	tv.tv_usec = usec % 1000000;
	if (select(0, NULL, NULL, NULL, &tv) < 0) {
		if (errno != EINTR) {
			perror("mcl_usleep: ERROR");
			mcl_exit(-1);
		}
	}
#endif /* OS_DEP */
}


/**
 * portable gettimeofday
 * @return returns 0 if ok, -1 if error, like the unix gettimeofday function
 */
int
mcl_gettimeofday (struct timeval	*time)
{
#ifdef WIN32
	UINT32 Gtime;
	Gtime = timeGetTime();
	time->tv_sec = Gtime/1000;
	time->tv_usec = (Gtime%1000)*1000;
	return 0;
#else /* UNIX */
	return gettimeofday(time, NULL);
#endif /* OS_DEP */
}


#ifdef SOLARIS
/**
 * inet_aton replacement for SOLARIS that doesn't include it.
 * feel free to comment if already available on your system...
 *
 * int inet_aton(const char *cp, struct in_addr *inp);
 *
 * inet_aton() converts the Internet host address cp from the
 * standard numbers-and-dots notation into  binary  data  and
 * stores  it  in the structure that inp points to. inet_aton
 * returns nonzero if the address is valid, zero if not.
 */
int inet_aton(const char *cp, struct in_addr *inp)
{
	// use a wrapup around: in_addr_t inet_addr(const char *cp);
	inp->s_addr = inet_addr(cp);
	if ((int)inp->s_addr == -1)
		return 0; /* failed, bad addr */
	else
		return 1; /* ok */
}
#endif // SOLARIS


/*
 *   MAD-ALC, implementation of ALC/LCT protocols
 *   Copyright (c) 2003 TUT - Tampere University of Technology
 *   main authors/contacts: jani.peltotalo@tut.fi and sami.peltotalo@tut.fi
 */ 
/**
 * This function converts double value to the closest integer value.
 * Usefull if the double value can have small calculation errors.
 * @params value	value to be converted.
 * @return		converted integer value.
 */
INT32
double_to_closest_int (double	value)
{
	double ceil_value;
	double floor_value;
	double abs_diff1;
	double abs_diff2;

	ceil_value = ceil(value);
	floor_value = floor(value);
	abs_diff1 = fabs(value - ceil_value);
	abs_diff2 = fabs(value - floor_value);
	if (abs_diff1 < abs_diff2) {
		return((INT32)ceil_value);
	} else {
		return((INT32)floor_value);
	}
}


#ifdef WIN32
/**
 * rint replacement for Windows that does not include it.
 * Prototype:
 *	double rint(double x);
 */
double
rint (double value)
{
	double ceil_value;
	double floor_value;
	double abs_diff1;
	double abs_diff2;

	ceil_value = ceil(value);
	floor_value = floor(value);
	abs_diff1 = fabs(value - ceil_value);
	abs_diff2 = fabs(value - floor_value);
	if (abs_diff1 < abs_diff2) {
		return(ceil_value);
	} else {
		return(floor_value);
	}
}
#endif // WIN32


/**
 * initialize srandom seed
 */
void
mcl_init_random ()
{
	struct timeval tmp = mcl_get_tvtime();	/* to improve randomness */

#ifdef WIN32
	srand(tmp.tv_usec);
#else
	srandom(tmp.tv_usec);
#endif
}


#ifdef WIN32
/**
 * Socket initialisation for WinSock
 */
void
mcl_winsock_init ()
{
	int err = 0;
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("mcl_winsock_init: Unable to initialize Winsock DLL (step 1)\n");
		mcl_exit(-1);
	}
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	if( LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 )
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		perror("mcl_winsock_init: Unable to initialize Winsock DLL (step 2)");
		WSACleanup( );
		mcl_exit(1);
	}
}

bool mcl_is_valid_sock (SOCKET sock)
{
	return ((sock == INVALID_SOCKET || (int)sock <= 0 ||
		 sock == SOCKET_ERROR) ? false : true);
}

#else  /* UNIX */

bool mcl_is_valid_sock (int sock)
{
	return ((sock <= 0) ? false : true);
}

#endif /* OS */
