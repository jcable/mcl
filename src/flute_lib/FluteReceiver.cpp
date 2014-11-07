/* $Id: FluteReceiver.cpp,v 1.5 2005/05/24 09:56:15 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
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

#include "flute_includes.h"

/**
 * FluteReceiver constructor
 */
FluteReceiver::FluteReceiver () {

	if ((flutecb->id = mcl_open("r")) < 0)
		EXIT(("FluteSender: ERROR, mcl_open(w) failed\n"))
		
	flutecb->mode = RECV;
	this->BytesReceived = 0;
	this->endOfRx_callback = NULL;
	this->receivedNewFileDescription_callback = NULL;

}

/**
 * FluteReceiver destructor
 */
FluteReceiver::~FluteReceiver () {
}

/**
 * Sets the Callback function called when reception is over.
 * @param EndOfRx_callback 	Callback function, having the number 
 *                              of received bytes as parameter.
 */
void FluteReceiver::setCallbackEndOfRx ( void* (*EndOfRx_callback)(UINT64 BytesRecvd)){
	
	this->endOfRx_callback = EndOfRx_callback;

}

/**
 * Sets the Callback function called when new file 
 * description is available.
 * @param EndOfRx_callback 	Callback function, having the 
 *                              file descirptions as parameter
 */
void FluteReceiver::setCallbackReceivedNewFileDescription (void* (*ReceivedNewFileDescription_callback) (class FluteFileInfo * fileInfo)) {

	this->receivedNewFileDescription_callback = ReceivedNewFileDescription_callback;

}

/**
 * Start receiving on the current Flute session.
 * Can be called in blocking or non-blocking mode.
 * @param blocking	call as blocking or not
 * @return		number of bytes received 
 */
UINT64 FluteReceiver::recv(bool blocking){

	UINT64 result;
	int mcl_option;
	
	flutecb->lock();
	
	if (flutecb->initialized == false)
	{
		/* finish initializing */
		this->init();
	}

	if (flutecb->storeAllAdu == true) {
		mcl_option = 1;
	} else {
		mcl_option = 0;
	}
	if (mcl_ctl(flutecb->id, MCL_OPT_SET_FLUTE_STORE_ALL_ADUS_BY_DEFAULT,
		    (void*)&mcl_option, sizeof(mcl_option))) {
		EXIT(("mcl_ctl: MCL_OPT_SET_FLUTE_STORE_ALL_ADUS_BY_DEFAULT failed\n"))
	}
	

	if (blocking == true) {

		flutecb->unlock();	
		this->waitRx((void *) NULL);
		flutecb->lock();	
	
	}
	else if (blocking == false) {
		flutecb->unlock();
#ifdef WIN32
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) this->waitRx((void *) NULL), (void*)NULL, 0, (LPDWORD)&reception_thread);
#else
		pthread_create(&reception_thread, NULL, (void*(*)(void*)) this->waitRx((void *) NULL), (void *)NULL);
		pthread_detach(reception_thread);
#endif	
		flutecb->lock();	
	}
	
	result = this->BytesReceived;
	
	flutecb->unlock();

	return result;
}

/**
 * Defines if a file should be overwritten or not:
 * possible parameters are:
 *  	PROMPT : prompt before overwriting an existing file,
 *	NEVER : never overwrite an existing file,
 *	ALWAYS : always overwrite existing file.
 * @param overwrite	NEVER, ALWAYS, or PROMPT
 */
void FluteReceiver::setOverwriteFile (int overwrite) {

	flutecb->lock();

	flutecb->overwrite = overwrite;

	flutecb->unlock();

}


/**
 * Sets the session source address.
 * @param src_addr	 source address
 */
void FluteReceiver::setSrcAddr (struct sockaddr *src_addr) {

	flutecb->lock();

	flutecb->src_addr = src_addr;

	flutecb->unlock();

}

/**
 * Sets the session source address.
 * @param source_addr_str	 source address
 */
void FluteReceiver::setSrcAddr (char	*source_addr_str) {

	flutecb->lock();

	if (flutecb->tools->extract_addr(source_addr_str, &flutecb->src_addr, 0) < 0) {
		EXIT(("Flute: setSrcAddr: ERROR, extract_addr failed\n" ))
	}

	flutecb->unlock();

}


/**
 * Set the reception directory (where files are written).
 * @param recv_dir	name of the reception dir
 */
void FluteReceiver::setRecvDir (char * recv_dir) {

	flutecb->lock();

	flutecb->unlock();

}


/**
 * Set/unset receiving all files.
 * @param on_off	true: recv all files,
 *			false: do not recv all files
 *			((un)select them individually with
 *			(un)selectTOI then).
 */
void FluteReceiver::setSelectAll (bool on_off) {

	flutecb->lock();

	flutecb->storeAllAdu = on_off;
	
	flutecb->unlock();

}

/**
 * Select a file for reception.
 * @param toi		toi of the file to receive.
 */
void FluteReceiver::selectTOI (TOI_t toi) {

	flutecb->lock();

	flutecb->fdt->selectTOI(flutecb->myfiles, toi);

	flutecb->unlock();

}

/**
 * Unselect a file for reception.
 * @param toi		toi of the file to not receive.
 */
void FluteReceiver::unselectTOI (TOI_t toi) {

	/*not supported yet*/

	flutecb->lock();
	flutecb->unlock();

}


/**
 * Flute receiving function (either called as a seperate thread
 * (non-blocking) or within the Flute-thread (blocking)).
 * @param arg		required for threads.
 */
void *FluteReceiver::waitRx(void * arg) {
	
	char		*buf_file = NULL;	/* buffer for recv'd fragment */
	UINT64		len = 0;		/* available object length */
						/* returned from GET_INFO */
	UINT64		len2 = 0;		/* actual object length after */
						/* mcl_recvmsg() call */
	UINT64		fdt_len;		/* actual FDT instance length */
	class FluteFile		*ThisFile = NULL;
	TOI_t 	toi;
	struct mcl_msghdr	mh;		/* for mcl_recvsmg */
	struct mcl_iovec	iov;		/* for mcl_recvsmg */

	/*
	 * Receive ALL objects...
	 * Here an object can be either a file or an FDT
	 */
	memset((void*)&mh, 0, sizeof(mh));
	while ((len = mcl_recvmsg(flutecb->id, &mh,
			  MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT)) != -1) {

		flutecb->lock();

		toi = (TOI_t) mh.toi;
#ifdef DEBUG
		PRINT(("Flute: New Object Received: %llu Bytes, TOI=%u\n", len, toi))
#endif
		if (toi == 0) {
			/*
			 * this is an FDT Instance, so receive the object
			 * in a buffer
			 */
			if (!(buf_file = (char*)malloc(len))) {
				EXIT(("Flute: Error: Cannot alloc memory!\n"))
			}
			/* a simple mcl_recv is sufficient here... */
			fdt_len = mcl_recv(flutecb->id, buf_file, len);
			if (fdt_len < 0 || fdt_len != len) {
				EXIT(("Flute: Error, mcl_recv failed for FDT, fdt_len=%llu\n", fdt_len))
			}
			flutecb->fdt->updateFDT(buf_file, len);

			if (flutecb->storeAllAdu == true)
				flutecb->fdt->selectAllTOIs(flutecb->myfiles);
			/* free this buffer, since next object may be
			 * significantly shorter or larger */
			free(buf_file);
			buf_file = NULL;

			/* call setCallbackReceivedNewFileDescription callback */
			if (this->endOfRx_callback != NULL) {

				class FluteFileInfo* fileinfo;
				fileinfo = flutecb->fdt->getFileInfoList();
				flutecb->unlock();
				this->receivedNewFileDescription_callback(fileinfo);
				flutecb->lock();
			
			}



		} else if ((ThisFile = flutecb->myfiles->FFileFindTOI(toi)) != NULL) {
			/*
			 * this is a known and selected file
			 */
			/*
			 * Content-Length Check, if this is specified in
			 * an FDT Instance (i.e. if filesize != 0)
			 */
			if ((ThisFile->contentLength != 0) && (len != ThisFile->contentLength)) {
				EXIT(("Flute: Error, actual length (%lld) different from length specified in FDT (%lld) for TOI %u\n", len, ThisFile->contentLength, toi))
			}
			if (ThisFile->writeIt) {
				/*
				 * this file can be written/over-written, so
				 * prepar the recvmsg call
				 */
				memset((void*)&mh, 0, sizeof(mh));
				mh.msg_iov = &iov;
				mh.msg_iovlen = 1;
				iov.iov_type = MCL_IOV_TYPE_FILE;
				iov.iov_base = NULL;
				iov.iov_filename = ThisFile->fullname;
				iov.iov_offset = (INT64)0;
				iov.iov_len = (INT64)len;
				len2 = mcl_recvmsg(flutecb->id, &mh, MCL_MSG_DEFAULT);
				if (len2 != len) {
					EXIT(("Flute: Error, mcl_recvmsg failed, returned %lld\n", len))
				}
				PRINT(("Flute: Received file \"%s\" (%lld Bytes)\n",
					ThisFile->fullname, len))
				ThisFile->received = 1;
#ifndef OPENSSL
				this->BytesReceived += len;
#else
				/*MD5sum checks*/
				if (strcmp((const char *)ThisFile->md5sum,"\0")==0) {
					PRINT(("Flute: No md5sum information for file %s\n", ThisFile->fullname));
					ThisFile->integrity = 0;
					this->BytesReceived += len;
				} else if (!FFilemd5sum_check(ThisFile)) {
					EXIT(("Flute: WARNING: wrong md5sum for file %s\n", ThisFile->fullname));
					//ThisFile->integrity=-1;
				} else {
					PRINT(("Flute: MD5sum OK, for file %s\n", ThisFile->fullname))
					ThisFile->integrity=1;			
					this->BytesReceived += len;
				}
#endif
			} else {
				PRINT(("Flute: Skipped file \"%s\"\n", ThisFile->fullname))
			}
		} else {
			/*
			 * this is an unknown or unwanted file
			 */
			PRINT(("Flute: WARNING: Received unknown object TOI %u\n",
				toi))
		}
		/* reset for next mcl_recvmsg */
		memset((void*)&mh, 0, sizeof(mh));
		
		/* Give some time to other function calls */
		flutecb->unlock();
	}
	
	
	if (this->endOfRx_callback != NULL) {
		
		UINT64 val = this->BytesReceived;
		flutecb->unlock();
		this->endOfRx_callback(val);
		flutecb->lock();
	
	}
	
	flutecb->unlock();
}
