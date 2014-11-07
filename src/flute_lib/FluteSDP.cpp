/* $Id: FluteSDP.cpp,v 1.3 2005/05/24 09:56:16 moi Exp $ */
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
 
/*
 * flute_sdp.cpp
 *
 *	Flute sdp parser.
 */
 
#include "flute_includes.h"



FluteSDP::FluteSDP (class flute_cb* flutecb)
{
	this->media_part = false;
	this->flutecb=flutecb;
}



FluteSDP::~FluteSDP ()
{
	this->media_part = false;
}



int flute_sdp_field_handler(
	SDP_Parser *   parser,
 	char           type,
 	const char *   value,
 	void *         user_data)
 {
	char * temp; /* temp string for string search operations */

 	switch(type) {
	case 'v':
	case 'o':
	case 'i':
	case 't':
		break;
	case 'a':
		if (!strncmp(value, "flute-tsi:", 10) && isdigit((int)*(value+10))) {
			((flute_cb *) user_data)->demux_label = atoi(value+10);
		}
		else if (!strncmp(value, "flute-ch:", 9) && isdigit((int)*(value+9))){
			((flute_cb *) user_data)->nb_layers = atoi(value+9);
		}
		else if (!strncmp(value, "source-filter:", 14) && isdigit((int)*(value+14))){
			if (((flute_cb *) user_data)->sdp->media_part == true) 
				/* source filters not allowed in media part */
				return SDP_FAILURE;

			if ((temp = strstr(value,"excl")) != NULL)
				/* exclusive source filters not allowed */
				return SDP_FAILURE;
				
			if ((temp = strstr(value,"IN")) == NULL)
				/* only IN source filters allowed */
				return SDP_FAILURE;

							
			if ((temp = strstr(value,"IP4")) != NULL)
			{
#ifdef WIN32
				((flute_cb *) user_data)->ip_version = 4;
#endif		
			}
			else if ((temp = strstr(value,"IP6")) != NULL)
			{
#ifdef WIN32
				((flute_cb *) user_data)->ip_version = 6;
#endif		

			}
			
			/* get the IP address string */
			temp = strstr(value,"* ");
			if (((flute_cb *) user_data)->tools->extract_addr(temp+2, &((flute_cb *) user_data)->src_addr, 0) < 0) {
					printf("Flute: ERROR, extract_addr failed for src_addr %s\n", temp+2);
					exit(-1);
			}


			if (!isdigit((int)*(temp+1)))
				return SDP_FAILURE;
			((flute_cb *) user_data)->port = atoi(temp+1);
			
		}
#if 0		
		else if (!strncmp(value, "FEC-declaration:", 16) && isdigit((int)*(value+16))){
			
		}
#endif
		break;
	case 'm':
		/* we are starting parsing the media part */
		if (((flute_cb *) user_data)->sdp->media_part == false) ((flute_cb *) user_data)->sdp->media_part = true;
		
		/* check if it is a FLUTE media type */
		temp = strstr(value,"FLUTE/UDP");
		if (temp == NULL)
			return SDP_FAILURE;
			
		/* jump to the port number */
		/* (Normally the line looks like: *
		 * m=application 12345 FLUTE/UDP 0 */
		temp = strstr(value," ");
		if (!isdigit((int)*(temp+1)))
			return SDP_FAILURE;
		((flute_cb *) user_data)->port = atoi(temp+1);
		break;
	case 'c':
		if ((temp = strstr(value,"IP4")) != NULL)
		{
#ifdef WIN32
			((flute_cb *) user_data)->ip_version = 4;
#endif		
		}
		else if ((temp = strstr(value,"IP6")) != NULL)
		{
#ifdef WIN32
			((flute_cb *) user_data)->ip_version = 6;
#endif		

		}
		else
			return SDP_FAILURE;
		
		/* get the IP address string */
		{
		char		*t,*c;
		strncpy(((flute_cb *) user_data)->session_addr_str, temp+4, sizeof(((flute_cb *) user_data)->session_addr_str));
		/* ttl */
		if ((t = strchr(((flute_cb *) user_data)->session_addr_str, '/')) != NULL) {
			*t = '\0';
			/* number of channels */
			if ((c = strchr(t, '/')) != NULL){
				*c = '\0';
			}
			/* in host order */
			((flute_cb *) user_data)->ttl = (unsigned short)atoi(t+1);
		}
		((flute_cb *) user_data)->session_addr_str[255] = '\0';
		}
		((flute_cb *) user_data)->session_address_set = true;
		
	default:
		break;
	}
	return SDP_SUCCESS;
 }
 
 
 
void FluteSDP::parseSDP(char * sdp_filename)
{
	SDP_Parser *sdp_parser;
	
	sdp_parser = SDP_NewParser();

	SDP_SetFieldHandler(sdp_parser, (SDP_FieldHandler) &flute_sdp_field_handler);
	SDP_SetUserData(sdp_parser, (void *) flutecb);
	
	if (SDP_EventStreamParseFile(sdp_parser, sdp_filename)!=true)
	{
		EXIT(("Flute: ERROR, SDP file seems not OK (or some features are not supported by our implementation)\n"))
	}

	SDP_DestroyParser(sdp_parser);
}



void FluteSDP::generateSDP(char * sdp_filename)
{
	char temp[256];
	SDP_Generator *generator = SDP_NewGenerator();

	SDP_GenProtocolVersionField(generator, 0);

	sprintf(temp,"%i",flutecb->demux_label);
	SDP_GenAttributeField(generator,"flute-tsi", temp);
	memset(temp,0, sizeof(temp));

	if(flutecb->nb_layers > 0)
	{
		sprintf(temp,"%i\n",flutecb->nb_layers);
		SDP_GenAttributeField(generator,"flute-ch", temp);
		memset(temp,0, sizeof(temp));
	}

	sprintf(temp,"%i",flutecb->port);	
	SDP_GenMediaDescriptionField(generator,"data",temp,"FLUTE/UDP","0");
	memset(temp,0, sizeof(temp));
	
	if (flutecb->session_addr != NULL)
	{
		if (flutecb->session_addr->sa_family == AF_INET)
		{
			SDP_GenConnectionField(generator,"IN", "IP4", inet_ntoa(((sockaddr_in*) flutecb->session_addr)->sin_addr),  flutecb->ttl,1);
		}
		else if (flutecb->session_addr->sa_family == AF_INET6)
		{
#ifdef WIN32
			DWORD len = INET6_ADDRSTRLEN;
			if (WSAAddressToString( (LPSOCKADDR) flutecb->session_addr, sizeof(struct sockaddr_in6), NULL, temp, &len)!=0)
			{
				EXIT(("Flute::flute_generate_sdp: ERROR, WSAStringToAddress failed: %i\n",  WSAGetLastError()))
			}

#else  /* UNIX */
			inet_ntop(AF_INET6, &(((struct sockaddr_in6*)flutecb->session_addr)->sin6_addr),temp, sizeof(temp));
#endif	
			SDP_GenConnectionField(generator,"IN", "IP6", temp,  flutecb->ttl,1);
		}
	}
	else
	{
		SDP_GenConnectionField(generator,"IN", "IP4", "127.0.0.1",  flutecb->ttl,1);
	}
	
	SDP_SaveGeneratedOutput(generator,sdp_filename);
}

