/* $Id: FluteFileInfo.h,v 1.2 2005/05/12 16:03:37 moi Exp $ */
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
 
#ifndef FLUTE_FILE_INFO_H
#define FLUTE_FILE_INFO_H


class FluteFileInfo {

public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	FluteFileInfo ();

	/**
	 * Default destructor.
	 */
	~FluteFileInfo ();

	FluteFileInfo* getNextFile();
	
	void getFilename(char ** filename, int* len);
	
	TOI_t getTOI();
	
	void getMIMEType(char ** type, int* len);
	
	UINT64 getBytesRcvd();
	
	UINT64 getTransferLength();
	
	UINT64 getContentLength();
	
	bool getReceived();
	
	INT32 getIntegrity();
	
	bool isSelected();

protected:

	FluteFileInfo * nextFile; // pointer to the next File (if any).
	
	char		fullname[MAX_PATH+MAX_FILENAME]; // Full path+name

	TOI_t	 	toi;		//toi of the file
	
	UINT64		contentLength;
	UINT64		transferLength;
	UINT64		bytesReceived;
	
	bool	received; // got the file??
	bool	selected;
	
	INT32	integrity;	// was md5sum check sucessfull or not? 
				// (-1 fail, 0 no information, 1 check ok)
	
	friend class FluteFile;
	friend class FluteFDT;

};

#endif
