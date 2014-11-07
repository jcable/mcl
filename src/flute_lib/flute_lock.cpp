/* $Id: flute_lock.cpp,v 1.2 2005/05/12 16:03:43 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  (main author: Christoph Neumann - christoph.neumann@inrialpes.fr
 *                Vincent Roca - vincent.roca@inrialpes.fr)
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

#include "flute_includes.h"


/**
 * initialize a mutex lock
 * @param mutex mutex to initialize
 */
void
flute_init_lock (flute_mutex_t	*mutex)
{
#if defined(WIN32)
	InitializeCriticalSection(mutex);
#elif defined(NO_PTHREAD_LOCK)
	/* Linux pthread lock not working with old ver*/
	*mutex = 0;
#else
	{
	pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;
	memcpy(mutex, &mutex_init, sizeof(pthread_mutex_t));
	}
#endif
}



/**
 * get this lock
 * @param mutex mutex lock
 */
void
flute_lock (flute_mutex_t *mutex)
{
#ifdef WIN32
	EnterCriticalSection(mutex);
#elif defined(NO_PTHREAD_LOCK)
	/* XXX: horrible synchro but have pbs with pthread_mutex_lock */
	while (*mutex > 0) {
		mcl_usleep(100);
	}
	(*mutex)++;
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
flute_trylock (flute_mutex_t *mutex)
{
#ifdef WIN32
	return 1;		/* non 0 to simulate error */
#elif defined(NO_PTHREAD_LOCK)
	/* XXX: horrible synchro but have pbs with pthread_mutex_lock */
	if (*mutex > 0) {
		return EBUSY;
	}
	(*mutex)++;
	return 0;
#else
	return pthread_mutex_trylock(mutex);
#endif /* OS_DEP */
}



/**
 * release a lock
 * @param mutex mutex lock
 */
void
flute_unlock (flute_mutex_t *mutex)
{
#ifdef WIN32
	LeaveCriticalSection(mutex);
#elif defined(NO_PTHREAD_LOCK)
	(*mutex)--;
#else
	pthread_mutex_unlock(mutex);
#endif /* OS_DEP */
}

