/* $Id: FluteFileInfo.cpp,v 1.3 2005/05/13 15:49:51 moi Exp $ */
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
 * FluteFileInfo constructor
 */ 
FluteFileInfo::FluteFileInfo (){

	this->nextFile = NULL;
	this->bytesReceived = 0;
}
 
/**
 * FluteFileInfo destructor
 */  
FluteFileInfo::~FluteFileInfo (){
}

/**
 * Get the next file in the list.
 * @return	next fileInfo in the list.
 */  
FluteFileInfo * FluteFileInfo::getNextFile(){

	return this->nextFile;

}

/**
 * Get the file name.
 * @param filename	(OUT) file name
 * @param len		(OUT) length of the string of the file name
 */  

void FluteFileInfo::getFilename(char ** filename, int* len) {

	*filename = fullname;
	*len = MAX_PATH+MAX_FILENAME;

}

/**
 * Get the TOI of the file
 * @return 	TOI of the file.
 */	
TOI_t FluteFileInfo::getTOI() {

	return this->toi;

}


/**
 * Get the MIME type of the file
 * @param type		(OUT) mime type (string)
 * @param len		(OUT) length of the string of the mime type
 */	
void FluteFileInfo::getMIMEType(char ** type, int* len) {
	/* TODO */
}


/**
 * Get the number of bytes received for that file
 * @return 	number of bytes received.
 */	
UINT64 FluteFileInfo::getBytesRcvd() {

	return bytesReceived;

}

/**
 * Get the transfer length.
 * @return 	transfer length.
 */	
UINT64 FluteFileInfo::getTransferLength() {

	return transferLength;

}

/**
 * Get the content length.
 * @return 	content length.
 */		
UINT64 FluteFileInfo::getContentLength() {

	return contentLength;
	
}

/**
 * Has the file been received entirly? Yes/No
 * @return 	true or false.
 */	
bool FluteFileInfo::getReceived() {

	return received;

}


/**
 * Has the file been selected for reception? Yes/No
 * @return 	true or false.
 */	
bool FluteFileInfo::isSelected() {

	return selected;

}

/** was md5sum check sucessfull or not? 
 * (-1 fail, 0 no information, 1 check ok)
 * @return 	-1 fail, 0 no information, 1 check ok
 */
int FluteFileInfo::getIntegrity() {

	return integrity;
	
}
