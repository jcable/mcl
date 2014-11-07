/* $Id: FluteSDP.h,v 1.2 2005/05/12 16:03:39 moi Exp $ */
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
 
#ifndef FLUTE_SDP_H
#define FLUTE_SDP_H

#include "../sdp_lib/src/SDP_Parser.h"
#include "../sdp_lib/src/SDP_Generator.h"


/**
 * The field sdp handler (called by the sdp parser 
 * when it encounters a field while parsing).
 * @param parser
 * @param type
 * @param value
 * @param user_data	 
 * @return 		SDP_FAILURE or SDP_SUCCESS
 */
int flute_sdp_field_handler(
	SDP_Parser *   parser,
	char           type,
	const char *   value,
	void *         user_data);


/**
 * This Class handels sdp files for flute
 */
class FluteSDP {
  
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	FluteSDP (class flute_cb * flutecb);

	/**
	 * Default destructor.
	 */
	~FluteSDP ();
 
	/**
	 * parses the sdp file and sets the appropriate parameters
	 */
	void parseSDP(char * sdp_filename);

	
	/**
	 * generates an sdp file
	 */
	void generateSDP(char * sdp_filename);

	/** are we parsing the media part or not */ 
	bool 	media_part;

private:

	/****** Private Attributes ********************************************/
	class flute_cb * flutecb;

};

#endif
