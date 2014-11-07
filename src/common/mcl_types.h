/* $Id: mcl_types.h,v 1.2 2005/04/08 15:23:31 moi Exp $ */
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

#ifndef MCL_TYPES_H
#define MCL_TYPES_H

/*
 * types
 */
#include <stdint.h>
#ifndef UINT32
#define INT8	int8_t
#define	INT16	int16_t
#define	UINT8	uint8_t
#define	UINT16	uint16_t
#define INT32	int32_t
#define	UINT32	uint32_t
#endif /* !UINT32 */

#if defined(WIN32)
#define INT64	LONGLONG
#define UINT64	ULONGLONG
#else  /* UNIX */
#define INT64	long long
#define UINT64	unsigned long long
#endif /* OS */

#define TOI_t 	UINT32

#endif /* MCL_TYPES_H */
