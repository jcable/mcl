/* $Id: mcl_file_io.h,v 1.3 2005/01/11 13:12:38 roca Exp $ */
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

#ifndef MCL_FILE_IO_H
#define MCL_FILE_IO_H


/**
 * Open the file whose name is provided, and associates a file descriptor
 * to it.
 * This function is clooser in spirit to open(2) (that returns a filedescr)
 * than to fopen(2) (that returns a stream). If a flag argument is provided,
 * note that other arguments (e.g. O_LARGEFILE) may be automatically added.
 * @param filename	file name
 * @param flags		one  of  O_RDONLY,  O_WRONLY or O_RDWR, and other
 * 			possible flags, as specified in open(2)
 * @param mode		specifies the permissions to use in case a new file
 *			is created, as specified in open(2)
 * @return		mcl_fopen returns the new file descriptor, or 
 * 			-1 if an error occurred.
 */
extern int	mcl_file_open (const char *filename, int flags, mode_t mode);

/**
 * Close a file descriptor, so that it no longer refers to any file.
 * This function is clooser in spirit to close(2) (that takes a filedescr)
 * than to fclose(3) (that takes a stream).
 * @param fd	file descriptor
 * @return	return zero on success, or -1 if an error occurred.
 */
extern int	mcl_file_close (int fd);

/**
 * Reposition read/write file offset.
 * The lseek function repositions the offset of the file of desciptor fd
 * to the argument offset. The offset is always considered from the begining
 * of the file.
 * @param fd
 * @param offset
 * @return
 *	Upon successful completion, lseek returns the resulting offset location
 *	as  measured  in  bytes  from  the beginning of the file.  Otherwise, a
 *	value of -1 is returned and errno is set to indicate the  error.
 */
extern INT64	mcl_file_lseek (int fd, INT64 offset);

/**
 * Read from a file descriptor.
 * Reads up to count bytes from the file referenced by the file
 * descriptor fd to the buffer starting at buf. 
 * 
 * @param fd
 * @param buf
 * @param count
 * @return
 *	On success, the number of bytes read is returned (zero indicates
 *	nothing was read). On error, -1 is returned and errno is set
 *	appropriately.
 */
extern INT64	mcl_file_read  (int fd, void *buf, INT64 count);

/**
 * Write to a file descriptor.
 * Writes  up  to  count  bytes  to the file referenced by the file
 * descriptor fd from the buffer starting at buf. 
 * 
 * @param fd
 * @param buf
 * @param count
 * @return
 *	On success, the number of bytes written is returned (zero indicates
 *	nothing was written). On error, -1 is returned and errno is set
 *	appropriately.
 */
extern INT64 mcl_file_write(int fd, const void *buf, INT64 count);

#endif /* MCL_FILE_IO_H */
