/* $Id: mcl_error.h,v 1.4 2005/01/11 13:12:27 roca Exp $ */
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

#ifndef MCL_ERROR_H  /* { */
#define MCL_ERROR_H

/*
 * error codes returned by the various functions 
 */

#define ERR_CORRUPT_HDR		-1000
#define ERR_BAD_LAYER		-1001
#define TOO_MANY_LOSSES		-1002
#define ERR_OPT_UNKNOWN		-1003
#define ERR_BAD_TRIGGER		-1004

#endif /* }  MCL_ERROR_H */
