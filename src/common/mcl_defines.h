/* $Id: mcl_defines.h,v 1.1 2004/06/07 09:44:50 roca Exp $ */
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

#ifndef MCL_DEFINES_H  /* { */
#define MCL_DEFINES_H


/****** general purpose enumerations ******************************************/

/**
 * Error status returned by functions
 */
enum mcl_error_status {
	MCL_OK	= 0,
	MCL_ERROR = 1
};


/****** general purpose macros ************************************************/

#ifndef min
#define min(a,b)	((a) <= (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)	((a) >= (b) ? (a) : (b))
#endif


/* WARNING: works only with integers */
#define is_odd(v)	( ((v) & 1) ? 1 : 0)


/****** constants: do not edit ************************************************/

/*
 * max length of char strings / paht+file names
 */
#define MAX_NAME_LEN		64
#define MAX_FILE_NAME_LEN	256


#endif /* } MCL_DEFINES_H */
