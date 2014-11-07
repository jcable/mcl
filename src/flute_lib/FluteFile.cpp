/* $Id: FluteFile.cpp,v 1.2 2005/05/12 16:03:36 moi Exp $ */
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
 
 
FluteFile::FluteFile(class flute_cb * flutecb) {	
	this->flutecb = flutecb;
}

FluteFile::~FluteFile() {	

	close(fd);
	
}

#ifdef OPENSSL
INT32 	FluteFile::FFilemd5sum_check() {

	unsigned char * digest = (unsigned char*)calloc(1,MD5BASE64_LENGTH);				
	int temp_fd=open(this->fullname, O_RDONLY);	
	md5sum_calculate(temp_fd, &digest);	
	close(temp_fd);
	
	if (strncmp((const char*)digest,(const char*) this->md5sum, MD5BASE64_LENGTH)==0) return 1;

	return 0;

}
#endif

class FluteFileInfo *FluteFile::createFileInfo() {

	class FluteFileInfo *NewFile = new FluteFileInfo();

	mcl_adu_rx_info_t rx_info; /* struct containing ADU reception info */
	rx_info.toi = this->toi;
	if (mcl_ctl(this->flutecb->id, MCL_OPT_GET_ADU_RX_INFO, (void*)&rx_info, sizeof(rx_info)) == 0) {
		NewFile->bytesReceived = rx_info.recvd_src_data;
	}
	strncpy(NewFile->fullname, this->fullname, MAX_PATH+MAX_FILENAME );
	NewFile->received = this->received;
	NewFile->selected = this->selected;
	NewFile->toi = this->toi;
	NewFile->integrity = this->integrity;
	NewFile->contentLength = this->contentLength;
	NewFile->transferLength	= this->transferLength;
	NewFile->nextFile = NULL;
	

	return NewFile;

}
