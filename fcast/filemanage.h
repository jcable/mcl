/* $Id: filemanage.h,v 1.1.1.1 2003/09/03 12:45:41 chneuman Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - Universite Paris 6 - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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
 * filemanage.h
 *
 *	Tools for Directories listing and files management...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>



#ifdef WIN32
#include <direct.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define MAX_FILENAME  512

#define PROMPT	1
#define NEVER	2
#define ALWAYS	3

typedef struct FFileStruct
{
	char	fullname[MAX_PATH+MAX_FILENAME];// Full path+name
	int		writeIt;		// Shall we write it or not?
	int		fd;				// File descriptor
	long	filesize;		// File size in Bytes
	int		nbFragTot;		// Total number of fragments for this file.
	int		nbFragRcvd;		// number of fragments already received.
	struct FFileStruct *next;	// pointer to the next File (if any).
} FFile, *pFFile, **ppFFile;


int			FileExist	( const char *filepath );
int			IsDirDots	( const char *path);
void		GetFileName	( char *filepath, char *filename );
void		GetFileBase	( char *filepath, char *filebase );
void 		GetFileBaseWithoutPrefix ( char *filepath, char *filebase );
int			CheckWriteContext ( char *filepath, int mode );

void	FFileInsert(ppFFile p_filelist, FFile newfile );
pFFile	FFileFind ( const char *fullname, pFFile filelist );
void	FFileRemove ( const char *fullname, ppFFile p_filelist );
void	FFilePrintList(pFFile FFList);

