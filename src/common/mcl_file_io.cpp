/* $Id: mcl_file_io.cpp,v 1.4 2005/03/23 14:05:04 roca Exp $ */
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

#ifdef ALC
#include "../alc/mcl_includes.h"

#elif defined(NORM)

#include "../norm/mcl_includes.h"
#endif // RM_PROTOCOL


int
mcl_file_open  (const char	*filename,
		int		flags,
		mode_t		mode)
{
	int	err;

	if ((err = open(filename, flags, mode)) < 0) {
		perror("mcl_file_open: ERROR");
	}
	return err;
}

int
mcl_file_close (int	fd)
{
	int	err;
	if ((err = close(fd)) < 0) {
		perror("mcl_file_close: ERROR");
	}
	return err;
}

/**
 * Reposition read/write file offset.
 */
INT64
mcl_file_lseek (int	fd,
		INT64	offset)
{
	return(lseek(fd, (off_t)offset, SEEK_SET));
}

/**
 * Read from a file descriptor.
 */
INT64
mcl_file_read  (int		fd,
		void		*buf,
		INT64		count)
{
	return(read(fd, buf, (size_t)count));
}

/**
 * Write to a file descriptor.
 */
INT64
mcl_file_write (int		fd,
		const void	*buf,
		INT64		count)
{
	return(write(fd, buf, (size_t)count));
}

