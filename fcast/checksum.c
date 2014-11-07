/* $Id: checksum.c,v 1.1.1.1 2003/09/03 12:45:41 chneuman Exp $ */
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
 * checksum.c
 *
 *	Checksum computing implementation.
 */

#include "checksum.h"
#include <stdio.h>

 
unsigned short ComputeSum_obsolete( unsigned short *Buffer, int count,  char *odd_boundary, int *nb_odd )
{
	register long sum = 0;
	unsigned short not_checksum = 0;
	unsigned short oddbyte = 0;
	char is_odd = (char)(count%2);
	if(is_odd && nb_odd != NULL) (*nb_odd) = (*nb_odd)+1;

	while( count > 1 ) /* This is the inner loop */
	{
		sum += *Buffer++;
		count -= 2;
	}
	if( count > 0 ) /* Add left-over byte, if any */
	{
		*((unsigned char *) &oddbyte) = *(unsigned char*)Buffer;
		sum += oddbyte;

 	}

	/* Fold 32bits sum to 16 bits */
	while ( sum >> 16 )
	{	
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	
	if( odd_boundary && *odd_boundary )
	{
		sum = ((sum & 0xFF00)>>8) + ((sum & 0x00FF)<<8);
	}

	if( odd_boundary )
		*odd_boundary = is_odd;
	
	not_checksum = (unsigned short)sum;
/*	PRINT(("<-ComputeSum: Sum=0x%lX)\n", not_checksum)) */
	return not_checksum;
}



unsigned short ComputeSum( unsigned short *Buffer, int count, unsigned char *swap )
{
	unsigned register long sum = 0;
	unsigned short not_checksum = 0;
	unsigned short oddbyte = 0;
	unsigned char inverse_swap = 0;

	while( count > 1 ) /* This is the inner loop */
	{
		sum += *Buffer++;
		count -= 2;
		if ( sum >> 16 )
		{	
			sum = (sum & 0xFFFF) + (sum >> 16);
		}
	}
	if( count > 0 ) /* Add left-over byte, if any */
	{
		*((unsigned char *) &oddbyte) = *(unsigned char*)Buffer;
		sum += oddbyte;
		inverse_swap = 1;
 	}

	/* Fold 32bits sum to 16 bits */
	while ( sum >> 16 )
	{	
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	
	if(swap && *swap)
	{
		sum = ((sum & 0xFF00)>>8) + ((sum & 0x00FF)<<8);
	}

	if(swap && inverse_swap)
		(*swap) = ((*swap)+1)%2;

	not_checksum = (unsigned short)sum;
	return not_checksum;
}


unsigned short ComputeChecksum( int sum, int swap)
{
	unsigned short checksum;

	if((swap))
	{
		sum = ((sum & 0xFF00)>>8) + ((sum & 0x00FF)<<8);
	}

	checksum = ~sum;
	return checksum;	
}
