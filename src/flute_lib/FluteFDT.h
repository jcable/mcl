/* $Id: FluteFDT.h,v 1.2 2005/05/12 16:03:35 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *		  Julien Laboure - julien.laboure@inrialpes.fr
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

#ifndef FluteFDT_H
#define FluteFDT_H

/* set it to 0 if FDT never expires */
#define FDT_LIFETIME 1000000

XERCES_CPP_NAMESPACE_USE

/**
 * This Class handels the fdt
 */
class FluteFDT {
  
public:	
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 * Creates the FDT 
	 */
	FluteFDT (class flute_cb * flutecb);
	
	/**
	 * Default destructor.
	 * Closes and frees the FDT
	 */
	~FluteFDT ();


	/**
	 * Sets the Expires attribute of the FDT.
	 * @param ivalue	character string of the timestamp value
	 */ 
	void setExpires(char * value);


	/**
	 * Returns the Expires attribute of the FDT.
	 * @return	character string of the Expires timestamp value
	 */ 
	char* getExpires();

	/**
	 * Check if the timestamp passed as character string expired,
	 * i.e. timestamp < time_now.
	 * @param ivalue	character string of the timestamp value
	 */
	void checkExpires(char * ivalue);


	/**
	 * Returns the Complete attribute of the FDT.
	 * @return	true if complete == true; else false.
	 */
	bool isComplete();


	/**
	 * Checks if object with a given toi is described in FDT.
	 * @param toi		TOI of the object
	 * @return		true if described in the FDT, else false.
	 */
	bool hasTOIinFDT(UINT64 toi);


	/**
	 * Update the FDT. Adds elements of the FDTinstance to the FDT
	 * @param xmlfdtinstance	pointer to the buffer containing the xml 
	 *				string of the fdt instance
	 * @param length		len of the xml string of the fdt instance
	 */
	void updateFDT(char* xmlfdtinstance, unsigned int length);


	/** 
	 * Creates a new File Element, that may be added to the FDT
	 * @return	the new File element
	 */
	XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* createNewFile();

	
	/**
	 * Appends a File Element to the FDT
	 * @param el	element that has to be appended to the FDT
	 */
	void appendFile(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *el);


	/**
	 * Sets value of an elements attribute. Attribute is created if needed.
	 * @param el		pointer to the element
	 * @param name		name of the attribute
	 * @param value		value of the attribute
	 * @return		the updated element el.
	 */
	XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* setAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* el, char * name, char* ivalue);


	/** 
	 * Gathers FTI information for a given node, and updates
	 * FTI_infos accordingly.
	 * @param nodeToCheck	node where to search FTI infos
	 * @param FTI_infos	struct that needs to be updated, containing all FTI information.
	 * @return		true if FTI information found, false if not.
	 */
	bool getFTIinfos(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*   nodeToCheck, FTI_infos_t * FTI_infos);


	/**
	 * Creates a new (empty) FDT instance, taking the FDT as a basis.
	 * @return	the FDT instance
	 */
	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* createNewFDTinstance();


	/**
	 * Adds a file to the FDT instance.
	 * @param fdtinstance	the FDT instance
	 * @param itoi		the toi of the file that has to added
	 * @return		the updated FDT instance
	 */
	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* AddFileToFDTinstance(XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* fdtinstance, unsigned int itoi);

	/**
	 * Creates the FDT instance xml-string.
	 * @param fdtinstance	the FDT instance
	 * @param buffer	outputbuffer containing the xml string, allocated by the callee
	 * @return		length of the xml string
	 */
	unsigned int getFinalFDTInstance(XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* fdtinstance, char ** buffer);


	/** 
	 * Adds the file with the given toi of the FDT 
	 * to the FileList and signal the selection to mclv3.
	 * @param filelist	list of files, where new has to be added to
	 * @param toi		toi of the file that has to be added
	 */
	void selectTOI(class FluteFileList *filelist, TOI_t toi);

	/** 
	 * Adds all elements of the FDT to the FileList 
	 * ONLY in !interactive mode.
	 * @param filelist	file list that needs to be updated
	 */
	void selectAllTOIs(class FluteFileList *filelist);


	class FluteFileInfo *getFileInfoList();

	class FluteFileInfo *getFileInfo(TOI_t toi);

private:

	 /*local variables*/
	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* doc;
	DOMImplementation *impl;
	DOMBuilder* dom_builder;
	
	UINT64 fdt_lifetime;
	class flute_cb * flutecb;


};

#endif











