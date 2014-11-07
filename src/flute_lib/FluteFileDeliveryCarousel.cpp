/* $Id: FluteFileDeliveryCarousel.cpp,v 1.3 2005/05/13 15:49:51 moi Exp $ */
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
 * FluteFileDeliveryCarousel constructor
 */
FluteFileDeliveryCarousel::FluteFileDeliveryCarousel(class FluteSender *flute) {
	
	this->flute = flute;
	this->flutecb = flute->flutecb;

	if(mcl_ctl(flutecb->id, MCL_OPT_KEEP_DATA, NULL, 0))
		EXIT(("Flute: mcl_ctl KEEP_DATA failed\n"))

	this->mode = READY;
	this->BytesSent = 0;
	this->endOfTxCycles_callback = NULL;

}

/**
 * FluteFileDeliveryCarousel destructor
 */ 
FluteFileDeliveryCarousel::~FluteFileDeliveryCarousel() {
}


/**
 * Reset the carousel:
 * - stop tx if carousel is in transmission
 * - remove all files scheduled for transmission
 */ 
void FluteFileDeliveryCarousel::reset() {

	/* TODO: reset the queued files have not been transmitted yet
	 */

	flutecb->lock();
	
	if (this->mode == IN_TX)
	{
		if(mcl_ctl(flutecb->id, MCL_OPT_RESET_TRANSMISSIONS, NULL, 0))
			EXIT(("Flute: mcl_ctl MCL_OPT_RESET_TRANSMISSIONS failed\n"))
	
		if(mcl_ctl(flutecb->id, MCL_OPT_KEEP_DATA, NULL, 0))
			EXIT(("Flute: mcl_ctl KEEP_DATA failed\n"))
	}
	flutecb->unlock();
	
}

/**
 * Sets the Callback function called when transmission is over.
 * @param EndOfTxCycles_callback 	Callback function, having the number 
 *                              	of transmitted bytes as parameter.
 */
void FluteFileDeliveryCarousel::setCallbackEndOfTxCycles(void* (*EndOfTxCycles_callback)(UINT64 BytesSent)) {

	flutecb->lock();
	this->endOfTxCycles_callback = EndOfTxCycles_callback;

	flutecb->unlock();
}

/**
 * Add file to be scheduled for transmission
 * @param filename	name of the file
 * @return		toi affected to the file
 */ 
TOI_t FluteFileDeliveryCarousel::addFile(char *filename) {


	TOI_t		result = 0;
	struct		stat file_stats;
	int		file_to_send 	= 0;
	struct mcl_iovec	iov;		/* for mcl_sendmsg */
	struct mcl_msghdr	msg;		/* for mcl_sendmsg */
#define TEMP_STRING_LEN	128
	char 		*temp_string;
	UINT64		len;
#ifdef OPENSSL
	unsigned char	*md5sum;
#endif
	XERCES_CPP_NAMESPACE_QUALIFIER DOMElement	*el;

	flutecb->lock();

	if (flutecb->initialized == false)
	{
	
		/* finish initializing */
		flute->init();
	
	}
	
	if (flutecb->fdtinstance == NULL) {
		/* create an fdt instance */
		flutecb->fdtinstance = flutecb->fdt->createNewFDTinstance();
	}

	/* allocate temp string for fdt instance attribute construction */
	if ((temp_string = (char*)malloc(TEMP_STRING_LEN)) == NULL) {
		EXIT(("Flute: SendThisFile: Error, malloc failed\n"))
	}


	/*
	 * Set the TOI for this file
	 */
	flutecb->toiindex++;
	if (mcl_ctl(flutecb->id, MCL_OPT_SET_NEXT_TOI, (void*)&flutecb->toiindex,
			sizeof(flutecb->toiindex)))
		EXIT(("Flute: mcl_ctl MCL_OPT_SET_NEXT_TOI failed\n"))


	if (FluteFile::FileExist(filename)) {
		if (stat(filename, &file_stats) == -1) {
			EXIT(("Flute: SendThisFile: Error: stat()\n"))
		}
#ifdef WIN32
		file_to_send = open(filename, O_RDONLY | O_BINARY);
#else
		file_to_send = open(filename, O_RDONLY);
#endif
		if (file_to_send < 0) {
			EXIT(("Flute: open failed for file %s\n", filename))
		}
	} else {
		PRINT(("Flute: SendThisFile: Error: %s, no such file!\n", filename))
		goto end;
	}
	if (!(file_stats.st_mode & S_IFREG))
		EXIT(("Flute: SendThisFile: Error: %s is not a regular file\n", filename))		

	flutecb->fec->ChooseFEC(file_stats.st_size);

	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_FILE;
	iov.iov_filename = filename;
	iov.iov_len = file_stats.st_size;
	if (mcl_sendmsg(flutecb->id, &msg, MCL_MSG_DEFAULT) < 0)
		EXIT(("Flute: SendThisFile: mcl_sendmsg failed\n"))
	this->BytesSent += file_stats.st_size;

	/*
	 * Update the FDT now...
	 */
	el = flutecb->fdt->createNewFile();

	/*TOI*/
#ifdef WIN32
	len = sprintf(temp_string, "%u", flutecb->toiindex);
#else
	len = snprintf(temp_string, TEMP_STRING_LEN, "%u", flutecb->toiindex);
#endif
	if (len < 0 || len >= TEMP_STRING_LEN) {
		EXIT(("Flute: SendThisFile: snprintf failed, returned %llu)\n", len))
	}
	el = flutecb->fdt->setAttribute(el, "TOI", temp_string);

	/*Content-Location*/
	el = flutecb->fdt->setAttribute(el, "Content-Location", filename);

	/*Content-Length*/
#ifdef WIN32
	len = sprintf(temp_string, "%lu", file_stats.st_size);
#else
	len = snprintf(temp_string, TEMP_STRING_LEN, "%lu", file_stats.st_size);
#endif
	if (len < 0 || len >= TEMP_STRING_LEN) {
		EXIT(("Flute: SendThisFile: snprintf failed, returned %llu)\n", len))
	}
	el=flutecb->fdt->setAttribute(el, "Content-Length", temp_string);

#ifdef OPENSSL
	/*Content-MD5*/
	if ((md5sum = (unsigned char*)calloc(1, MD5BASE64_LENGTH)) == NULL) {
		EXIT(("Flute: SendThisFile: Error, calloc failed"))
	}
	md5sum_calculate(file_to_send, &md5sum);	
	el = flutecb->fdt->setAttribute(el, "Content-MD5", (char *)md5sum);
	free(md5sum);
#endif

	flutecb->fdt->appendFile(el);

	/* Adding to fdt-instance*/
	flutecb->fdt->AddFileToFDTinstance(flutecb->fdtinstance, flutecb->toiindex);

	close(file_to_send);
	
	if (this->mode == IN_TX) {	
	
		/*
		 * ...and the associated FDT.
		 */
		char	*fdtbuffer;
		int	fdtsize = 0;
		int	mcl_option_toi = 0;	// set TOI to 0 for the FDT

		/*fdtbuffer is allocated in getFinalFDTInstance*/
		fdtsize = flutecb->fdt->getFinalFDTInstance(flutecb->fdtinstance, &fdtbuffer);
		if (mcl_ctl(flutecb->id, MCL_OPT_SET_NEXT_TOI, (void*)&mcl_option_toi,
			sizeof(mcl_option_toi))) {
			EXIT(("Flute: mcl_ctl MCL_OPT_SET_NEXT_TOI failed\n"))
		}
		if (mcl_ctl(flutecb->id,  MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE, (void*)NULL, 0)) {
			EXIT(("Flute: mcl_ctl MCL_OPT_SET_NEXT_TOI failed\n"))
		}
		/* Now flush high priority objects, to be sure that the FDT instance 
		   will be sent entirely before all the ojects it is describing */
		if (mcl_ctl(flutecb->id,  MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS, (void*)NULL, 0)) {
			EXIT(("Flute: mcl_ctl MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS failed\n"))
		}

		flutecb->fec->ChooseFEC(fdtsize);

		if (mcl_send(flutecb->id, fdtbuffer , fdtsize) < 0)
			EXIT(("Flute: mcl_send failed\n"))
			
		if (fdtbuffer!= NULL) free(fdtbuffer);	
		flutecb->fdtinstance->release();
		flutecb->fdtinstance = NULL;	

	}
	
	free(temp_string);

end:
	result =  flutecb->toiindex;
	flutecb->unlock();
	return result;
}


/**
 * Remove file from transmission schedule.
 * @param toi		toi of the files that has to be removed
 */ 
void FluteFileDeliveryCarousel::removeFile(TOI_t toi) {

	flutecb->lock();

	/* TODO: remove files that haven't been transmitted yet.
	 */
	 
	if (this->mode == IN_TX) {
		if (mcl_ctl(flutecb->id, MCL_OPT_STOP_TRANSMITTING_ADU, (void *)&toi, sizeof(toi)))
			EXIT(("Flute: mcl_ctl MCL_OPT_STOP_TRANSMITTING_ADU failed toi %ul\n", toi))
	}
	flutecb->unlock();

}


/**
 * Start a new transmission cycle, indicating the number of cycles.
 * Can be called either in blocking (wait until the end of transmission
 * in that case) or non blocking mode;
 * @param nb_cycles		number of transmission cycles, or CONTINUOUS
 * @param blocking		call as blocking or not
 * @return			number of transmitted bytes.
 */ 
UINT64 FluteFileDeliveryCarousel::startTxCycles(INT32 nb_cycles, bool blocking) {

	UINT64 result = 0;

	flutecb->lock();
	
	flutecb->nb_tx = nb_cycles;
	if (flutecb->nb_tx == CONTINUOUS)
	{
		flutecb->delivery_mode = DEL_MODE_ON_DEMAND;
	}
	else if (flutecb->nb_tx > 0)
	{
		flutecb->delivery_mode = DEL_MODE_PUSH;
		if (flutecb->nb_tx >= 1) {
			if( mcl_ctl(flutecb->id, MCL_OPT_NB_OF_TX, (void*)&nb_cycles,
					sizeof(nb_cycles)))
				EXIT(("Flute: ERROR, mcl_ctl failed for NB_OF_TX\n"))
	}

	}
	else
		EXIT(("Flute: ERROR, non valid number of Tx cycles %i\n",nb_cycles))

	if(mcl_ctl(flutecb->id, MCL_OPT_DELIVERY_MODE, (void*)&flutecb->delivery_mode,
						sizeof(flutecb->delivery_mode)))
		EXIT(("Flute: ERROR, mcl_ctl failed for DELIVERY_MODE\n"))
	

	/*
	 * ...and the associated FDT.
	 */
	if (flutecb->fdtinstance != NULL)
	{
		char	*fdtbuffer;
		int	fdtsize = 0;
		int	mcl_option_toi = 0;	// set TOI to 0 for the FDT

		/*fdtbuffer is allocated in getFinalFDTInstance*/
		fdtsize = flutecb->fdt->getFinalFDTInstance(flutecb->fdtinstance, &fdtbuffer);
		if (mcl_ctl(flutecb->id, MCL_OPT_SET_NEXT_TOI, (void*)&mcl_option_toi,
			sizeof(mcl_option_toi))) {
			EXIT(("Flute: mcl_ctl MCL_OPT_SET_NEXT_TOI failed\n"))
		}
		if (mcl_ctl(flutecb->id,  MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE, (void*)NULL, 0)) {
			EXIT(("Flute: mcl_ctl MCL_OPT_SET_NEXT_TOI failed\n"))
		}
		/* Now flush high priority objects, to be sure that the FDT instance 
		   will be sent entirely before all the ojects it is describing */
		if (mcl_ctl(flutecb->id,  MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS, (void*)NULL, 0)) {
			EXIT(("Flute: mcl_ctl MCL_OPT_FLUSH_HIGH_IMPORTANCE_OBJECTS failed\n"))
		}

		flutecb->fec->ChooseFEC(fdtsize);

		if (mcl_send(flutecb->id, fdtbuffer , fdtsize) < 0)
			EXIT(("Flute: mcl_send failed\n"))		
	
		if (fdtbuffer!= NULL) free(fdtbuffer);	
		flutecb->fdtinstance->release();
		flutecb->fdtinstance = NULL;
	}
	
	if(mcl_ctl(flutecb->id, MCL_OPT_PUSH_DATA, NULL, 0))
		EXIT(("Flute: mcl_ctl PUSH_DATA failed\n"))
	
	this->mode = IN_TX;

	if (blocking == true) {
	
		flutecb->unlock();
		this->waitTx((void *) NULL);
		flutecb->lock();
	
	}
	else if (blocking == false) {
		flutecb->unlock();
#ifdef WIN32
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) this->waitTx((void *) NULL), (void*)NULL, 0, (LPDWORD)&send_thread);
#else
		pthread_create(&send_thread, NULL, (void*(*)(void*)) this->waitTx((void *) NULL), (void *)NULL);
		pthread_detach(send_thread);
#endif	
		flutecb->lock();	
	}
	
	result = BytesSent;
	flutecb->unlock();
	
	return result;


}



/**
 * Add new transmission cycles to an ongoing carousel.
 * @param nb_cycles		number of transmission cycles, or CONTINUOUS
 */ 
void FluteFileDeliveryCarousel::addTxCycles(INT32 nb_cycles) {

	flutecb->lock();
	
	if (nb_cycles == CONTINUOUS)
	{
		flutecb->nb_tx = nb_cycles;
		flutecb->delivery_mode = DEL_MODE_ON_DEMAND;
	}
	else if (nb_cycles > 0)
	{
		flutecb->delivery_mode = DEL_MODE_PUSH;

		flutecb->nb_tx += nb_cycles;
		if (flutecb->nb_tx >= 1) {
			if( mcl_ctl(flutecb->id, MCL_OPT_ADD_NB_OF_TX, (void*)&nb_cycles,
					sizeof(nb_cycles)))
				EXIT(("Flute: ERROR, mcl_ctl failed for NB_OF_TX\n"))
	}

	}
	else
		EXIT(("Flute: ERROR, non valid number of Tx cycles %i\n",nb_cycles))

	if(mcl_ctl(flutecb->id, MCL_OPT_DELIVERY_MODE, (void*)&flutecb->delivery_mode,
						sizeof(flutecb->delivery_mode)))
		EXIT(("Flute: ERROR, mcl_ctl failed for DELIVERY_MODE\n"))
	
	flutecb->unlock();

}

/**
 * Stop an ongoing transmission carousel.
 * @return		number of transmitted bytes
 */ 
UINT64 FluteFileDeliveryCarousel::stopTx() {

	flutecb->lock();

	if (this->mode == IN_TX)
	{
		if(mcl_ctl(flutecb->id, MCL_OPT_RESET_TRANSMISSIONS, NULL, 0))
			EXIT(("Flute: mcl_ctl MCL_OPT_RESET_TRANSMISSIONS failed\n"))
	
		if(mcl_ctl(flutecb->id, MCL_OPT_KEEP_DATA, NULL, 0))
			EXIT(("Flute: mcl_ctl KEEP_DATA failed\n"))
	}

	flutecb->unlock();

	return BytesSent;

}


/**
 * Flute transmission function (either called as a seperate thread
 * (non-blocking) or within the Flute-thread (blocking)).
 * @param arg		required for threads.
 */
void *FluteFileDeliveryCarousel::waitTx(void * arg) {
	
	if (mcl_wait_event(flutecb->id, MCL_WAIT_EVENT_END_TX) == -1)
		EXIT(("Flute: mcl_wait_event MCL_WAIT_EVENT_END_TX failed\n"))
	
	flutecb->lock();
	
	if (this->endOfTxCycles_callback != NULL) {
		
		UINT64 val = this->BytesSent;
		flutecb->unlock();
		this->endOfTxCycles_callback(val);
		flutecb->lock();
	}
	
	flutecb->unlock();
}
