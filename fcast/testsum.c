/* $Id: testsum.c,v 1.1.1.1 2003/09/03 12:45:42 chneuman Exp $ */
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
 *	Test program for checksum generations
 */
#include <stdio.h>
#include <stdlib.h>
#include "checksum.h"


 int main(int argc, char *argv[])
 {
	unsigned int i = 0;
	unsigned char *block1 = NULL;
	unsigned char *block2 = NULL;
	unsigned char *block3 = NULL;
	unsigned short not_checksum = 0;
	unsigned short checksum = 0;
	char must_swap = 0;
	unsigned int size = 0;
	unsigned char abyte = 0;
	int strip = 0;
	unsigned int read = 0;


	printf("\nTestSum v1.0b (c)J. LABOURE, 2ooo\n=> Compute and test an Internet Checksum\n\n");

	read1:	printf("Number of bytes? ");
	fflush(stdin);
	scanf("%d", &size);
	if( size < 1 )
	{
		printf("Error: invalid number!\n");
		goto read1;
	}

	printf("Strip at: ");
	fflush(stdin);
	scanf("%d", &strip);
	if(strip<=0 || strip > (int)size)
	{
		printf("No striping...\n");
	 	if (!(block1 = (char *)malloc(size)))
		{
			printf("Error: Cannot alloc memory!\n");
			return -1;
		}

		for( i=0; i< size; i++ )
		{
			read2: printf("Content for byte %d: ", i+1);
			fflush(stdin);
			scanf("%X", &read);
			if( read> 0xFF )
			{
				printf("Error: invalid byte value!\n");
				goto read2;
			}
			abyte = (unsigned char) read;
			printf("  Read 0x%X\n", abyte);

			*(block1+i) = abyte;		
		}

	 	not_checksum = ComputeSum( (unsigned short*)block1, size, NULL );
		checksum = ~not_checksum;
		if (checksum == 0)
			printf("Checksum OK\n");
		printf("Checksum %d\n", checksum);
	}
	else
	{
	 	if (  !( block1 = (char *)malloc(strip) ) || !( block2 = (char *)malloc(size-strip) ) || !( block3 = (char *)malloc(4) )  )
		{
			printf("Error: Cannot alloc memory!\n");
			return -1;
		}

		for( i=0; i< strip; i++ )
		{
			read22: printf("Content for byte %d: ", i+1);
			fflush(stdin);
			scanf("%X", &read);
			if( read> 0xFF )
			{
				printf("Error: invalid byte value!\n");
				goto read22;
			}
			abyte = (unsigned char) read;
			printf("  Read 0x%X\n", abyte);

			*(block1+i) = abyte;		
		}
		printf("--- Stripping Here! ---\n");

		for( i=strip; i< size; i++ )
		{
			read222: printf("Content for byte %d: ", i+1);
			fflush(stdin);
			scanf("%X", &read);
			if( read> 0xFF )
			{
				printf("Error: invalid byte value!\n");
				goto read222;
			}
			abyte = (unsigned char) read;
			printf("  Read 0x%X\n", abyte);

			*(block2+i-strip) = abyte;		
		}


	 	* (unsigned short*)block3 = ComputeSum( (unsigned short*)block1, strip , &must_swap);
	 	* (unsigned short*)(block3+2) = ComputeSum( (unsigned short*)block2, size-strip , &must_swap);
		
		not_checksum = ComputeSum( (unsigned short*)block3, 4, NULL );
		checksum = ~not_checksum;
		if (checksum == 0)
			printf("Checksum OK\n");
		printf("\nDone! Global Sum=0x%X, Checksum=0x%X\n", not_checksum, checksum);
		
	}

 	return 1;
}
