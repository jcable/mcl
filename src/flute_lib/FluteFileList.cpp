/* $Id: FluteFileList.cpp,v 1.2 2005/05/12 16:03:38 moi Exp $ */
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
 
 
FluteFileList::FluteFileList (class flute_cb *flutecb) {

	this->flutecb = flutecb;
	this->head = NULL;

}

FluteFileList::~FluteFileList () {

	FluteFile *temp;
	
	/* free the list */
	while(this->head != NULL) {
		
		temp = this->head;
		this->head = this->head->nextFile;
		delete temp;
	}

}


void	FluteFileList::FFileInsert(FluteFile *newfile) {

	FluteFile *list = this->head;

	if (list == NULL) {
		
		this->head = newfile;
	}	
	else
	{
		while (list->nextFile != NULL) {
			list = list->nextFile;
		}
		list->nextFile = newfile;
	}

}

FluteFile *FluteFileList::FFileFind ( const char *fullname) {

	FluteFile * found = NULL;
	FluteFile * listloop = this->head;
	
	while(listloop != NULL)
	{
		if( !strcmp(listloop->fullname, fullname) ) {
			found = listloop;
			break;
		}
		listloop = listloop->nextFile;
	}
	return found;

}

FluteFile *FluteFileList::FFileFindTOI ( TOI_t toi ) {

	FluteFile * found = NULL;
	FluteFile * listloop = this->head;
	
	while(listloop != NULL)
	{
		if(listloop->toi==toi) {
			found = listloop;
			break;
		}
		listloop = listloop->nextFile;
	}
	return found;

}

void	FluteFileList::FFileRemove (const char *fullname) {


	FluteFile * found = this->FFileFind(fullname);
	FluteFile * listloop = this->head;

	if (found != NULL && listloop != NULL)
	{
		if(listloop == found)
		{
			this->head = listloop->nextFile;
			delete listloop;
		}
		else
		{
			while(listloop->nextFile != found)
			{
				listloop = listloop->nextFile;
			}

			listloop->nextFile = found->nextFile;
			delete found;
		}
	}
	else
	{
		EXIT(("Error: FFile %s not found!\n", fullname))
	}


}


void	FluteFileList::FFileRemoveTOI (TOI_t toi) {

	FluteFile * found = this->FFileFindTOI(toi);
	FluteFile * listloop = this->head;

	if (found != NULL && listloop != NULL)
	{
		if(listloop == found)
		{
			this->head = listloop->nextFile;
			delete listloop;
		}
		else
		{
			while(listloop->nextFile != found)
			{
				listloop = listloop->nextFile;
			}

			listloop->nextFile = found->nextFile;
			delete found;
		}
	}
	else
	{
		EXIT(("Error: FFile TOI %lu not found!\n", toi))
	}


}
