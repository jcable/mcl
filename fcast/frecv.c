/* $Id: frecv.c,v 1.11 2005/03/23 14:05:00 roca Exp $ */
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
 * frecv.c
 *
 *	fcast receiver side functions
 */
#include "fcast.h"


void FcastRecv(void)
{
	int Bytes_received = 0;
//	int received = 0;
#if defined(ALC)
	int mcl_option;

	if (optimode == OPTIMIZE_SPACE) {
		mcl_option = 1;
		if (mcl_ctl(id, MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, (void*)&mcl_option,
				sizeof(mcl_option)))
			EXIT(("mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI failed\n"))
		mcl_option = 0;
		if (mcl_ctl(id, MCL_OPT_POSTPONE_FEC_DECODING,
				(void*)&mcl_option, sizeof(mcl_option))) {
			/* non critical... ignore! */
			//EXIT(("mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING failed\n"))
		}

	} else if (optimode == OPTIMIZE_SPEED) {
		mcl_option = 1;
		if (mcl_ctl(id, MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, (void*)&mcl_option,
				sizeof(mcl_option)))
			EXIT(("mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI failed\n"))
		mcl_option = 0;
		if (mcl_ctl(id, MCL_OPT_POSTPONE_FEC_DECODING,
				(void*)&mcl_option, sizeof(mcl_option))) {
			/* non critical... ignore! */
			//EXIT(("mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING failed\n"))
		}
	} else if (optimode == OPTIMIZE_CPU) {
		mcl_option = 0;
		if (mcl_ctl(id, MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI, (void*)&mcl_option,
				sizeof(mcl_option)))
			EXIT(("mcl_ctl: MCL_OPT_IMMEDIATE_DELIVERY_TO_APPLI failed\n"))
		mcl_option = 1;
		if (mcl_ctl(id, MCL_OPT_POSTPONE_FEC_DECODING,
				(void*)&mcl_option, sizeof(mcl_option))) {
			/*
			 * non critical... ignore!
			 * for instance, this is only valid if RSE is used,
			 * but does not apply to LDPC/LDGM
			 */
			//EXIT(("mcl_ctl: MCL_OPT_POSTPONE_FEC_DECODING failed\n"))
		}
	} else {
		EXIT(("FATAL ERROR: invalid optimization mode!"))
	}
#endif /* RM_PROTOCOL */

	if (src_addr > 0) {
		/* in host format! */
		if (mcl_ctl(id, MCL_OPT_SRC_ADDR, (void*)&src_addr,
				sizeof(src_addr)))
			EXIT(("mcl_ctl: MCL_OPT_SRC_ADDR failed\n"))
	}

	PRINT(("Waiting for data...\n"))

	/* and now receive all... */
	Bytes_received = RecvFiles();

	mcl_close(id);
	PRINT(("\nFcastRecv completed, %d bytes received\n", Bytes_received))
}


int RecvFiles (void)
{
	char		*buf_file = NULL;	/* buffer for recv'd fragment */
	unsigned char	BlockTmp4[4];
	char		file_path[MAX_FILENAME + MAX_PATH];
	int			len		= 0;
	char		*Trailer	= NULL;
	unsigned short	global_sum	= 0;
	unsigned long	trailerlength	= 0;
	meta_t		*MetaList	= NULL;
	meta_t		*lpMetaData	= NULL;
	int		CurrFrag = 0;
	int		offset = 0;
	int		received = 0;
	long		FragLength = 0;
	pFFile		PartialFiles = NULL;
	pFFile		ThisFile;
#if defined(NORM)
	int		max_fragment_size; /* Big files are fragmented into
					      fragments of this size */
#elif defined(ALC)
	struct mcl_msghdr	mh;		/* for mcl_recvsmg */
#endif

	/*
	 * Receive _ALL_ objects...
	 */
#if defined(NORM)
	/* recvmsg is not yet implemented, so use a static buffer of
	 * maximum object length */
	/* determine the maximum fragment size (pessimistic evaluation that
	 * does not take the FEC actually used into account) */
	max_fragment_size = RSE_MAX_FRAGMENT_SIZE;
	//max_fragment_size = max(max_fragment_size, LDGM_MAX_FRAGMENT_SIZE);
	max_fragment_size = max(max_fragment_size, NULL_FEC_MAX_FRAGMENT_SIZE);

	/* Objects contain a file (or fragment), with its associated trailer
	 * and checksum... */
	if(!(buf_file = (char*)malloc(max_fragment_size + MAX_TRAILER_SIZE))) {
		EXIT(("Fcast ERROR: Cannot alloc memory!\n"))
	}

	/* Receiving all objects... */
	while((len = mcl_recv(id, buf_file,
			      max_fragment_size + MAX_TRAILER_SIZE)) != -1) {

#elif defined(ALC)

	/* recvmsg is used to identify the exact object length */
	memset((void*)&mh, 0, sizeof(mh));
	while ((len = mcl_recvmsg(id, &mh,
			  MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT)) != -1) {
		/* Objects contain a file (or fragment), with its associated
		 * trailer and checksum... */
		if(!(buf_file = (char*)malloc(len))) {
			EXIT(("Fcast ERROR: Cannot alloc memory (%d bytes needed)!\n", len))
		}
		len = mcl_recv(id, buf_file, len);
		if (len < 0) {
			EXIT(("Fcast ERROR: mcl_recv failed\n"))
		}
#endif /* RM_PROTOCOL */
#ifdef DEBUG
		PRINT(("New Object Received (%d Bytes)\n", len))
#endif

		/* Compute the checksum and check if GlobalSum is 0xFFFF
		 * (i.e. checksum == 0) */
		global_sum = ComputeSum( (unsigned short*)buf_file, len, NULL );
		if ( global_sum != 0xFFFF) {
			//EXIT(("Fcast ERROR: Invalid Checksum!!! (sum 0x%X)\n", global_sum))
			printf("Fcast ERROR: Invalid Checksum!!! (sum 0x%X)\n", global_sum);
		}

		/* Get the Trailer Infos */
		memcpy( BlockTmp4, (buf_file+len-8), 4);
		trailerlength = BUFFER_TO_INT32(BlockTmp4);
#ifdef DEBUG
		PRINT(("\n)=> Trailer infos: %ld Bytes.\n", trailerlength))
#endif

		if (!(Trailer = (char*)malloc(trailerlength + 1))) {
			EXIT(("Error: Cannot alloc memory!\n"))
		}
		memcpy( Trailer, (buf_file +len -8 -trailerlength), trailerlength );
		Trailer[trailerlength]= '\0';

		/* Parse All Metas from the Trailer */
		MetaList = ParseTrailer(Trailer);
		free(Trailer);
		GetFilePathFromMeta(MetaList, file_path);

		if( (FragLength = GetMetaLength(MetaList)) != (long)(len -8 -trailerlength) )
		{
			EXIT(("Fcast ERROR: Size of fragment received and \"Content-length\" value don't match! (%ld!=%ld)\n",lpMetaData->value, GetMetaLength(MetaList)))
		}

		FFilePrintList(PartialFiles);
		
		ThisFile = FFileFind(file_path, PartialFiles);

		if (ThisFile == NULL) {
			/*
			 * First fragment for this FILE
			 */
			FFile NewFile;
			strncpy(NewFile.fullname, file_path, MAX_PATH+MAX_FILENAME );
#ifdef DEBUG
			PRINT(("New File received: %s\n", NewFile.fullname))
#endif
			PrintMetaList(MetaList);
			NewFile.writeIt = CheckWriteContext(NewFile.fullname, overwrite);
			if(NewFile.writeIt)
			{
#ifdef WIN32
				if ((NewFile.fd = open(NewFile.fullname,
						O_WRONLY | O_CREAT | O_BINARY | O_TRUNC, _S_IWRITE)) < 0)
#else
				if ((NewFile.fd = open(NewFile.fullname,
						O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
#endif
				{
					EXIT(("Error while opening file \"%s\"\n", NewFile.fullname))
				}
			}
			NewFile.filesize = GetMetaFilesize(MetaList);
			NewFile.nbFragRcvd = 1;
			NewFile.next = NULL;
			GetMetaFragment(MetaList, &CurrFrag, &NewFile.nbFragTot);
			FFileInsert(&PartialFiles, NewFile);
			ThisFile = FFileFind(file_path, PartialFiles);
		} else {
			/*
			 * We've already received fragment(s) from this file
			 */
			int nbfrag; 
			ASSERT(!strcmp(ThisFile->fullname, file_path))
#ifdef DEBUG
			PRINT(("File %s already in database...\n",
				ThisFile->fullname))
#endif
			/* ASSERT(ThisFile->fd != 0) */
			ASSERT(ThisFile->filesize == GetMetaFilesize(MetaList))
			ASSERT(ThisFile->nbFragRcvd >0 )
			GetMetaFragment(MetaList, &CurrFrag, &nbfrag);
			ASSERT(ThisFile->nbFragTot == nbfrag)
			ThisFile->nbFragRcvd++;
		}

		ASSERT(ThisFile != NULL)
		ASSERT(ThisFile->nbFragRcvd<= ThisFile->nbFragTot)

		if (ThisFile->writeIt) {
			PRINT(("Processing fragment %d of file %s (Total=%d / Rcvd=%d)\n", CurrFrag, ThisFile->fullname, ThisFile->nbFragTot, ThisFile->nbFragRcvd))
			offset = GetMetaOffset(MetaList);
#ifdef DEBUG
			PRINT(("\tWriting %ld bytes at offset %d\n", FragLength, offset))
#endif

			if (lseek((int)ThisFile->fd, (long)offset, SEEK_SET) < 0)
				EXIT(("Error: lseek failed\n"))
			if (write((int)ThisFile->fd, buf_file, FragLength) < 0)
				EXIT(("mclrecv: write failed\n"))

			if (ThisFile->nbFragRcvd == ThisFile->nbFragTot)
			{
				// This File is complete!
				PRINT(("Finished receiving file \"%s\" (%ld Bytes).\n", ThisFile->fullname, ThisFile->filesize))
				ASSERT(ThisFile->fd != 0)
				close(ThisFile->fd);
				FFileRemove(ThisFile->fullname, &PartialFiles);
			}
			received += len;

		} else {
			/*
			 * This is a skipped file
			 */
			PRINT(("Skipping fragment %d of file %s (Total=%d / Rcvd=%d) (file already exists)\n", CurrFrag, ThisFile->fullname, ThisFile->nbFragTot, ThisFile->nbFragRcvd))

			if (ThisFile->nbFragRcvd == ThisFile->nbFragTot) {
				/*
				 * This File is complete!
				 */
				PRINT(("File \"%s\" -> All fragments have been skipped (file already exists).\n", ThisFile->fullname))
				//ASSERT(ThisFile->fd == 0)
				FFileRemove(ThisFile->fullname, &PartialFiles);
			}
			
		}
		DestroyMetalist(MetaList);
#ifdef ALC
		/* free this buffer, since next object may be significantly
		 * shorter or larger */
		free(buf_file);
		buf_file = NULL;
#endif
	}

	if (PartialFiles != NULL) {
		// When loop ends, all files must be completed!
		FFilePrintList(PartialFiles);
		EXIT(("Fcast ERROR: MCL session closed and there are missing fragments\n"))
		// TODO display missing files/fragment
	}

	return received;
}

