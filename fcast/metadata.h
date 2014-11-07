/* $Id: metadata.h,v 1.1.1.1 2003/09/03 12:45:42 chneuman Exp $ */
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "macros.h"

#define MAX_NAMELEN	128
#define MAX_CONTLEN	256


typedef struct trailer_struct
{
	unsigned long size;
	char * buffer;
}trailer_t;
	

typedef struct meta_struct
{
	char name[MAX_NAMELEN]; 	/* The meta name */
	char content[MAX_CONTLEN];	/* The meta content */
	long value; 	/* The meta content value if it's a number */
	struct meta_struct *next; /* the next meta in the trailer, or NULL */
}meta_t;


void	AddMetadata(trailer_t *trailer, char *MetaName, char *MetaContent);
void	EndTrailer(trailer_t *trailer);
void	DeleteTrailer(trailer_t *trailer);


meta_t*	ParseTrailer(char *TrailerBuff);
	/* warning: TrailerBuff is modified by ParseTrailer */
meta_t* FindMeta(meta_t *MetaList, char *MetaName);
void	DestroyMetalist(meta_t *MetaList);
void	PrintMetaList(meta_t *MetaList);

void	GetFilePathFromMeta(meta_t *MetaList, char *FullFilePath);
long	GetMetaLength(meta_t *MetaList);
long	GetMetaFilesize(meta_t *MetaList);
long	GetMetaOffset(meta_t *MetaList);
int	GetMetaFragment(meta_t *MetaList, int *FragIndice, int *NbFragTot);

