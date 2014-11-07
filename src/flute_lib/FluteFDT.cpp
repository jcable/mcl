/* $Id: FluteFDT.cpp,v 1.2 2005/05/12 16:03:34 moi Exp $ */
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

#include "flute_includes.h"

/**
 * Creates the FDT 
 */
FluteFDT::FluteFDT(class flute_cb * flutecb)
{
	XMLCh tempStr[100];
	
	this->flutecb=flutecb;
	
	try
	{
		XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during initialization:\n\t %s\n", message))
        }	

	/* First I get a DOMImplementation reference */	
        XMLString::transcode("XML 1.0 XML 2.0 Range 2.0 Traversal 2.0", tempStr, 99);
        impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
	
    /* Initialize the dom parser */
    dom_parser = ((DOMImplementationLS*)impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

	/* I create a new document with FDT as root element */
	XMLString::transcode("FDT", tempStr, 99);
        doc = impl->createDocument(0, tempStr, 0);
	
	fdt_lifetime = FDT_LIFETIME;
	
	/* Now handle the expires attribute */
	if (fdt_lifetime == 0)
	{
		/* set the maximum expiration time */
		setExpires("4294967294");
	}
	else
	{
		time_t systime;
		char * str_time = (char *) calloc(1,10); //10 is maximum length of UL string
		UINT64 currenttime;

		time(&systime);
		currenttime = (UINT64) systime + (UINT64) 2208988800;
	
#ifdef WIN32
		sprintf(str_time,"%I64u",currenttime + fdt_lifetime);
#else
		sprintf(str_time,"%llu",currenttime + fdt_lifetime);
#endif
		setExpires(str_time);
		free(str_time);
	}
	
}

/** 
 * Closes and frees the FDT 
 */
FluteFDT::~FluteFDT()
{	

 	/* I free the document structure */
	doc->release();

	XMLPlatformUtils::Terminate();
}


/**
 * Checks if object with a given toi is described in FDT.
 * @param toi		TOI of the object
 * @return		true if described in the FDT, else false.
 */
bool FluteFDT::hasTOIinFDT(UINT64 toi){

	XMLCh tempStr[100];
	unsigned int i, nchilds;
	bool hasTOI = false;

	try
	{
	  	/* I get reference to the root element of the document */
 		DOMElement*   root = doc->getDocumentElement();
			
		XMLString::transcode("File", tempStr, 99);
		DOMNodeList*  node_list = root->getElementsByTagName(tempStr);
		
		nchilds = node_list->getLength();
		XMLString::transcode("TOI", tempStr, 99);
		for (i = 0; i < nchilds; i++) {
			DOMNode* node = node_list->item(i);
			DOMAttr*    pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
			const XMLCh*        toi_value = pAttr->getValue();
			if(toi==(UINT64) XMLString::parseInt(toi_value))
			{
				hasTOI=true;
				break;	
			}
		}
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during hasTOIinFDT(UINT64 toi):\n\t %s\n", message))
        }	

	return hasTOI;

}


/**
 * Sets value of an elements attribute. Attribute is created if needed.
 * @param el		pointer to the element
 * @param name		name of the attribute
 * @param value		value of the attribute
 * @return		the updated element el.
 */
DOMElement*  FluteFDT::setAttribute(DOMElement* el, char * name, char* ivalue)
{
	XMLCh tempStr[100];

	try
	{
		/*Get the attribute*/
 		XMLString::transcode(name, tempStr, 99);
		DOMAttr*      pAttr;
		if ((pAttr = el->getAttributeNode(tempStr))== NULL)
		{
			pAttr = doc->createAttribute(tempStr);
	       	 	el->setAttributeNode(pAttr);
		}
			
		/* fix the URI if it is the Content-Location attribute */
		if (strncmp(name,"Content-Location",16)==0)
		{
			XMLCh tempStr2[100];
			XMLCh tempStr3[100];
			XMLString::transcode(ivalue, tempStr3, 99);
			XMLString::fixURI(tempStr3, tempStr2);
			pAttr->setValue((const XMLCh *)tempStr2);
		}
		else if (strncmp(name,"Expires",7)==0)
		{
			/* we need this since otherwise transcode 
			 * transforms the string into a false integer string
			 */
			pAttr->setValue((const XMLCh *)ivalue);
		}
		else
		{
			XMLString::transcode(ivalue, tempStr, 99);
			pAttr->setValue((const XMLCh *)tempStr);
		}
		
		
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during setAttribute(DOMElement* el, char * name, char* ivalue):\n\t %s\n", message))
        }
	
	return el;
} 


/**
 * Check if the timestamp passed as character string expired,
 * i.e. timestamp < time_now.
 * @param ivalue	character string of the timestamp value
 */
void FluteFDT::checkExpires(char * ivalue)
{
	time_t systime;
	time(&systime);
	
#ifdef WIN32
	if ((UINT64) systime + (UINT64) 2208988800 >= _atoi64(ivalue))
#else
	if ((UINT64) systime + (UINT64) 2208988800 >= strtoull(ivalue,NULL,10))
#endif
	{
	    char * str_time = (char *) malloc(10); //10 is maximum length of UL string
	    sprintf(str_time,"%llu", (UINT64) systime);
            free(str_time);
            EXIT(("FDT Instance expired: Expires at %s and it is %s\n", ivalue , str_time))
	}
	
}

 
/**
 * Sets the Expires attribute of the FDT.
 * @param ivalue	character string of the timestamp value
 */ 
void FluteFDT::setExpires(char * ivalue)
{
	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();
	
	setAttribute(root, "Expires", ivalue);
}

/**
 * Returns the Expires attribute of the FDT.
 * @return	character string of the Expires timestamp value
 */ 
char * FluteFDT::getExpires()
{	
	XMLCh tempStr[100];
	char * result;

	try
	{ 
  		/* I get reference to the root element of the document */
 		DOMElement*   root = doc->getDocumentElement();
 
		/*Get the expires attribute*/
 		XMLString::transcode("Expires", tempStr, 99);
		DOMAttr*      pAttr = root->getAttributeNode(tempStr);
	
		const XMLCh* 	      value = pAttr->getValue();
 	
		result = (char *) value;
	}
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during getExpires():\n\t %s\n", message))
        }

	return result;
}

/**
 * Returns the Complete attribute of the FDT.
 * @return	true if complete == true; else false.
 */
bool FluteFDT::isComplete()
{	
	XMLCh tempStr[100];

	try
	{ 
  		/* I get reference to the root element of the document */
 		DOMElement*   root = doc->getDocumentElement();
 
		/*Get the expires attribute*/
 		XMLString::transcode("Complete", tempStr, 99);
		DOMAttr*      pAttr = root->getAttributeNode(tempStr);

		if (pAttr == NULL) return false;

		const XMLCh* 	      value = pAttr->getValue();
		
		if (value == false) return false;
		
	}
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during isComplete():\n\t %s\n", message))
        }

	return true;
}



 
 
/**
 * Update the FDT. Adds elements of the FDTinstance to the FDT
 * @param xmlfdtinstance	pointer to the buffer containing the xml 
 *				string of the fdt instance
 * @param length		len of the xml string of the fdt instance
 */
void FluteFDT::updateFDT(char* xmlfdtinstance, unsigned int length)
{
	XMLCh tempStr[100];

	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();

	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* fdtinstance;
	
	try {

#if 0  /* validate the fdt_instance, doesn't seem to work. TODO: fixit*/	
		/* First validate the xml file using the FLUTE Schema */
		SAXParser* parser = new SAXParser;
		parser->setValidationScheme(SAXParser::Val_Always);
		parser->setDoNamespaces(true);
		parser->setDoSchema(true);
		parser->setDoValidation(true);
		parser->setValidationSchemaFullChecking(true);
		parser->setValidationConstraintFatal(true);
		parser->setExternalNoNamespaceSchemaLocation("FLUTE_SCHEMA.xsd");
		parser->setExternalSchemaLocation("FLUTE_SCHEMA.xsd");
		parser->parse(*(new MemBufInputSource((const XMLByte *) xmlfdtinstance,length, tempStr)));
		delete parser;
#endif

		/*Then build the DOM tree*/
        const DOMLSInput* domIS = new Wrapper4InputSource(new MemBufInputSource((const XMLByte *) xmlfdtinstance,length, tempStr));
        fdtinstance = dom_parser->parse(domIS);
		delete domIS;

        }
        catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
	        EXIT(("Exception message is: %s\n", message))
	}
	
  	/* I get reference to the root element of the document */
 	DOMElement*   fdt_instance_root;
	if ((fdt_instance_root = fdtinstance->getDocumentElement()) == NULL)
	{
		EXIT(("Root element is null\n"))
	}

	/* Check Expires node */
 	XMLString::transcode("Expires", tempStr, 99);
	DOMAttr*      pAttr = fdt_instance_root->getAttributeNode(tempStr);
	const XMLCh* 	      value = pAttr->getValue();
	char * temp_char = XMLString::transcode(value);
	checkExpires(temp_char);
	free(temp_char);
	
	/* Go through the attributes of the instance root element and check if it is already included in the FDT */
	DOMNamedNodeMap *  node_map = fdt_instance_root->getAttributes();
	long i,nattr;
	nattr = node_map->getLength();
	for (i = 0; i < nattr; i++) {
		DOMNode* node = node_map->item(i);
		if (node->getNodeType() == DOMNode::ATTRIBUTE_NODE)
		{
			const XMLCh* name;
			char * temp_char2;
			value = ((DOMAttr*) node)->getValue();
			name = ((DOMAttr*) node)->getName();
			temp_char = XMLString::transcode(value);
			temp_char2 = XMLString::transcode(name);
			setAttribute(root, temp_char2, temp_char);
			free(temp_char2);
			free(temp_char);
		}
	}
	
	/* Go through the child-nodes elements of the instance and check if it is already included in the FDT */	
	DOMNodeList*  node_list = fdt_instance_root->getChildNodes();

	long nchilds;
	nchilds = node_list->getLength();
	XMLString::transcode("TOI", tempStr, 99);
	for (i = 0; i < nchilds; i++) {
		DOMNode* node = node_list->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
			value = pAttr->getValue();
			if(! hasTOIinFDT((UINT64) XMLString::parseInt(value)) )
			{
				DOMNode* new_node = doc->importNode(node,true);
				root->appendChild(new_node);
			}
		}
	}
	fdtinstance->release();
}


/** 
 * Adds the file with the given toi of the FDT 
 * to the FileList and signal the selection to mclv3.
 * @param filelist	list of files, where new has to be added to
 * @param toi		toi of the file that has to be added
 */

void FluteFDT::selectTOI(class FluteFileList *filelist, TOI_t toi)
{
	XMLCh tempStr[100];
	DOMNode*   node = NULL;
	unsigned int i, nchilds;	

  	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();
	
	XMLString::transcode("File", tempStr, 99);
	DOMNodeList*  node_list = root->getElementsByTagName(tempStr);
		
	nchilds = node_list->getLength();
	XMLString::transcode("TOI", tempStr, 99);
	for (i = 0; i < nchilds; i++) {
		DOMNode* temp_node = node_list->item(i);
		DOMAttr*    pAttr = ((DOMElement *) temp_node)->getAttributeNode(tempStr);
		const XMLCh*        toi_value = pAttr->getValue();
		if(toi==(unsigned int) XMLString::parseInt(toi_value))
		{
			node = temp_node;
			break;	
		}
	}
	
	if (node == NULL) {
		return;
	}
	else
	{
		class FluteFile *NewFile = new FluteFile(this->flutecb);
		
		/* Content-Location */
		XMLString::transcode("Content-Location", tempStr, 99);
		DOMAttr*      pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
		const XMLCh* 	      value = pAttr->getValue();
		char * temp_char = XMLString::transcode(value);
		
		/* Remove string file:// */
		if (XMLString::startsWith(temp_char,"file://"))
		{
			int len = strlen(temp_char);
			strncpy(temp_char, temp_char + 7, len - 7);
			temp_char[len - 7] = '\0';
		}
		
		/* Detect possible ':' (like c:/) and replace them with '/' */
		char *p;
		if ((p = strchr(temp_char, ':')) != NULL) {
			*p = '/';
		}

		strncpy(NewFile->fullname,"./", 2); /* just to be sure it is copied in current directory */
		strncpy(NewFile->fullname+2, temp_char, MAX_PATH+MAX_FILENAME );
		NewFile->writeIt = FluteFile::CheckWriteContext(flutecb, NewFile->fullname, flutecb->overwrite);
		NewFile->received=0;
		NewFile->selected=1;
		NewFile->toi = toi;
		NewFile->integrity =0;
		NewFile->nextFile = NULL;
		
		free(temp_char);
		
		/* Content-Length */
		NewFile->contentLength=0;
		XMLString::transcode("Content-Length", tempStr, 99);
		if ((pAttr = ((DOMElement *) node)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			NewFile->contentLength = (unsigned long) XMLString::parseInt(value);
		}
		
		/*We assume transferLength = contentLength */
		NewFile->transferLength = NewFile->contentLength;
		
#if OPENSSL
		/* Content-MD5 */
		XMLString::transcode("Content-MD5", tempStr, 99);
		memset(NewFile->md5sum, 0, MD5BASE64_LENGTH);
		if ((pAttr = ((DOMElement *) node)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			char * temp_char2 = XMLString::transcode(value);
			strncpy((char *)NewFile->md5sum ,temp_char,MD5BASE64_LENGTH);
			free(temp_char2):
		}				
#else
		memset(NewFile->md5sum, 0, MD5BASE64_LENGTH);
		//ZeroMemory(NewFile.md5sum, MD5BASE64_LENGTH);
#endif	
		filelist->FFileInsert(NewFile);

		/* Now signal it to mclv3 */
		{
			int mcl_option = NewFile->toi;
			if (mcl_ctl(flutecb->id, MCL_OPT_FLUTE_DELIVER_THIS_ADU_TO_APPLI, (void*)&mcl_option, sizeof(mcl_option)))
				EXIT(("mcl_ctl: MCL_OPT_FLUTE_DELIVER_THIS_ADU_TO_APPLI failed\n"))
			
			/* Get FTI info if there is ... */	
			
			FTI_infos_t 	FTI_infos;
			memset(&FTI_infos, 0, sizeof(FTI_infos));
			
			FTI_infos.toi = NewFile->toi;
			FTI_infos.adu_len = NewFile->contentLength;
			
			/* First check if there is FTI info at File element node then at FDT-Instance root */
			if ((getFTIinfos(node, &FTI_infos) == true) || (getFTIinfos(root, &FTI_infos) == true))
			{	
				if (mcl_ctl(flutecb->id, MCL_OPT_SET_FTI_INFO, (void*)&FTI_infos, sizeof(FTI_infos)))
					EXIT(("mcl_ctl: MCL_OPT_SET_FTI_INFO failed\n"))
			}
		}


		
	}
		
}

/** 
 * Adds all elements of the FDT to the FileList 
 * ONLY in !interactive mode.
 * @param filelist	file list that needs to be updated
 */
void FluteFDT::selectAllTOIs(class FluteFileList *filelist)
{
	
	XMLCh tempStr[100];
	
	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();

	/* Go through the nodelist*/	
	DOMNodeList*  node_list = root->getChildNodes();

	long i, nchilds;
	nchilds = node_list->getLength();
	XMLString::transcode("TOI", tempStr, 99);
	for (i = 0; i < nchilds; i++) {
		DOMNode* node = node_list->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			DOMAttr*    pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
			const XMLCh* 	   value = pAttr->getValue();
			if(((filelist->FFileFindTOI(XMLString::parseInt(value)))==NULL))
			{
				selectTOI(filelist, XMLString::parseInt(value));						
			}
		}
	}
} 



/** 
 * Gathers FTI information for a given node, and updates
 * FTI_infos accordingly.
 * @param nodeToCheck	node where to search FTI infos
 * @param FTI_infos	struct that needs to be updated, containing all FTI information.
 * @return		true if FTI information found, false if not.
 */
bool FluteFDT::getFTIinfos(DOMNode*   nodeToCheck, FTI_infos_t * FTI_infos)
{
	XMLCh tempStr[100];
	DOMAttr*      pAttr;
	const XMLCh*  value;

	try
	{ 		
			
		/* FEC-OTI-FEC-Encoding-ID */
		XMLString::transcode("FEC-OTI-FEC-Encoding-ID", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->fec_encoding_id = (UINT32) XMLString::parseInt(value);
		}
		else return false;
		
		
		/* FEC-OTI-FEC-Instance-ID */
		/* is not always transmitted (only for underspecified schemes) */
		XMLString::transcode("FEC-OTI-FEC-Instance-ID", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->fec_instance_id = (UINT16) XMLString::parseInt(value);
		}	
		else if (FTI_infos->fec_encoding_id > 127) 
			/* FEC-Instance-ID is required for underspecified schemes */
			return false;


		/* FEC-OTI-Maximum-Source-Block-Length */
		XMLString::transcode("FEC-OTI-Maximum-Source-Block-Length", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->max_k = (UINT32) XMLString::parseInt(value);
		}
		else return false;
		

		/* FEC-OTI-Encoding-Symbol-Length */
		XMLString::transcode("FEC-OTI-Encoding-Symbol-Length", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->symbol_len = (UINT16) XMLString::parseInt(value);
		}
		else return false;	
			
				
		/* FEC-OTI-Max-Number-of-Encoding-Symbols */
		/* is not always transmitted (depending on the FEC scheme) */
		XMLString::transcode("FEC-OTI-Max-Number-of-Encoding-Symbols", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->max_n = (UINT32) XMLString::parseInt(value);
		}
		
		
		/* FEC-OTI-Seed */
		/* is not always transmitted (depending on the FEC scheme) */
		XMLString::transcode("FEC-OTI-Seed", tempStr, 99);
		if ((pAttr = ((DOMElement *) nodeToCheck)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			FTI_infos->fec_key = (UINT32) XMLString::parseInt(value);
		}

        } catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            EXIT(("Error during getFTIinfos():\n\t %s\n", message))
        }

	return true;

}


/** 
 * Creates a new File Element, that may be added to the FDT
 * @return	the new File element
 */

DOMElement* FluteFDT::createNewFile()
{

	XMLCh tempStr[100];

	XMLString::transcode("File", tempStr, 99);
	DOMElement*     el = doc->createElement(tempStr);
	
	return el;
}


/**
 * Appends a File Element to the FDT
 * @param el	element that has to be appended to the FDT
 */

void FluteFDT::appendFile(DOMElement *el)
{

	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();

	/* append the node */
 	root->appendChild (el);

}

/**
 * Creates a new (empty) FDT instance, taking the FDT as a basis.
 * @return	the FDT instance
 */
XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* FluteFDT::createNewFDTinstance()
{ 	


	XMLCh tempStr1[100];
	XMLCh tempStr2[100];	

	/* I create a new document with FDT as root element */
	XMLString::transcode("FDT-Instance", tempStr1, 99);
    XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument*    fdtinstance = impl->createDocument(0, tempStr1, 0);
	
	/* I get reference to the root element of the document */
 	DOMElement*   root = fdtinstance->getDocumentElement();

	/* I add the Expires attribute to the element  */	
	XMLString::transcode("Expires", tempStr1, 99);
	XMLString::transcode(getExpires(), tempStr2, 99);
	root->setAttribute (tempStr1, tempStr2);
		
	return fdtinstance;
}


/**
 * Adds a file to the FDT instance.
 * @param fdtinstance	the FDT instance
 * @param itoi		the toi of the file that has to added
 * @return		the updated FDT instance
 */
XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* FluteFDT::AddFileToFDTinstance(XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* fdtinstance ,unsigned int itoi)
{

	XMLCh tempStr[100];
	
	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();

	/* Go through the nodelist*/	
	DOMNodeList*  node_list = root->getChildNodes();

	long i, nchilds;
	nchilds = node_list->getLength();
	XMLString::transcode("TOI", tempStr, 99);
	for (i = 0; i < nchilds; i++) {
		DOMNode* node = node_list->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			DOMAttr*    pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
			const XMLCh*        toi_value = pAttr->getValue();
			if(itoi==(unsigned int) XMLString::parseInt(toi_value))
			{
				/* I get reference to the root element of the document */
			 	DOMElement*   root_instance = fdtinstance->getDocumentElement();
				DOMNode* new_node = fdtinstance->importNode(node,true);
				root_instance->appendChild(new_node);
				
			}

		}
	}
	
	return fdtinstance;
	
}

/**
 * Creates the FDT instance xml-string.
 * @param fdtinstance	the FDT instance
 * @param buffer	outputbuffer containing the xml string, allocated by the callee
 * @return		length of the xml string
 */
unsigned int FluteFDT::getFinalFDTInstance(XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* fdtinstance, char ** buffer)
{	
	XMLCh tempStr2[100];
	unsigned int len = 0;

	if (fdtinstance == NULL)
	{
			return 0;
	}

    DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	XMLString::transcode("UTF-8", tempStr2, 99);
    DOMLSOutput* theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();
    theOutputDesc->setEncoding(tempStr2);

	MemBufFormatTarget *  target = new MemBufFormatTarget(FLUTE_MAX_FDT_SIZE);
    theOutputDesc->setByteStream(target);

    theSerializer->write(fdtinstance, theOutputDesc);

	len = target->getLen();
	
	*buffer = (char*) malloc(len);
	
	memcpy(*buffer, (char*) target->getRawBuffer(), len);
	
	theSerializer->release();
	delete target;	
	
	return len;
}


class FluteFileInfo *FluteFDT::getFileInfoList()
{

	class FluteFileInfo * returnedList = NULL;
	class FluteFileInfo * returnedListElement = NULL;

	XMLCh		tempStr[100];
	unsigned int	i; /* current index while crossing the DOM node list. */
	unsigned int	nchilds;

	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();
	DOMNodeList*  node_list = root->getChildNodes();
	if ((nchilds = node_list->getLength())==0) {	
		return NULL;
	}
	/*
	 * Now go through all entries in the DOM node list...
	 */
	for (i = 0; i < nchilds; i++) {
		DOMNode* node = node_list->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
			XMLString::transcode("TOI", tempStr, 99);
			DOMAttr*	pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
			const XMLCh*	toi_value = pAttr->getValue();
			const unsigned int toi = XMLString::parseInt(toi_value);
		
			if (returnedList == NULL) 
				returnedList = returnedListElement = this->getFileInfo(toi);
			else {
				returnedListElement->nextFile = this->getFileInfo(toi);
				returnedListElement = returnedListElement->nextFile;
			}
		}
	}

	
	
	
	return returnedList;
}


class FluteFileInfo *FluteFDT::getFileInfo(TOI_t toi)
{	
	class FluteFileInfo *NewFileInfo = NULL;
	class FluteFile *temp_NewFile = NULL;
	XMLCh tempStr[100];
	DOMNode*   node = NULL;
	unsigned int i, nchilds;	

	/* first check if it isn't already included in the filelist */
	if ( (temp_NewFile = flutecb->myfiles->FFileFindTOI(toi)) != NULL) 
	{
		return temp_NewFile->createFileInfo();
	}


  	/* I get reference to the root element of the document */
 	DOMElement*   root = doc->getDocumentElement();
	
	XMLString::transcode("File", tempStr, 99);
	DOMNodeList*  node_list = root->getElementsByTagName(tempStr);
		
	if ((nchilds = node_list->getLength())==0) {	
		return NULL;
	}
	XMLString::transcode("TOI", tempStr, 99);
	for (i = 0; i < nchilds; i++) {
		DOMNode* temp_node = node_list->item(i);
		DOMAttr*    pAttr = ((DOMElement *) temp_node)->getAttributeNode(tempStr);
		const XMLCh*        toi_value = pAttr->getValue();
		if(toi==(unsigned int) XMLString::parseInt(toi_value))
		{
			node = temp_node;
			break;	
		}
	}
	
	if (node == NULL) {
		return NULL;
	}
	else
	{
		NewFileInfo = new FluteFileInfo();
		
		/* Content-Location */
		XMLString::transcode("Content-Location", tempStr, 99);
		DOMAttr*      pAttr = ((DOMElement *) node)->getAttributeNode(tempStr);
		const XMLCh* 	      value = pAttr->getValue();
		char * temp_char = XMLString::transcode(value);
		
		/* Remove string file:// */
		if (XMLString::startsWith(temp_char,"file://"))
		{
			int len = strlen(temp_char);
			strncpy(temp_char, temp_char + 7, len - 7);
			temp_char[len - 7] = '\0';
		}
		
		/* Detect possible ':' (like c:/) and replace them with '/' */
		char *p;
		if ((p = strchr(temp_char, ':')) != NULL) {
			*p = '/';
		}

		strncpy(NewFileInfo->fullname,"./", 2); /* just to be sure it is copied in current directory */
		strncpy(NewFileInfo->fullname+2, temp_char, MAX_PATH+MAX_FILENAME );
		NewFileInfo->received=0;
		NewFileInfo->toi = toi;
		NewFileInfo->integrity =0;
		NewFileInfo->nextFile = NULL;
		
		/* Content-Length */
		NewFileInfo->contentLength=0;
		XMLString::transcode("Content-Length", tempStr, 99);
		if ((pAttr = ((DOMElement *) node)->getAttributeNode(tempStr))!=NULL)
		{
			value = pAttr->getValue();
			NewFileInfo->contentLength = (unsigned long) XMLString::parseInt(value);
		}
		
		/*We assume transferLength = contentLength */
		NewFileInfo->transferLength = NewFileInfo->contentLength;
		
	}

	return NewFileInfo;

}

