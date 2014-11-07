/* $Id: FluteFec.h,v 1.2 2005/05/12 16:03:35 moi Exp $ */
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

#ifndef FLUTE_FEC_H
#define FLUTE_FEC_H



/* available FEC codecs and associated information */
typedef struct fec_codec_info {
	int	available;		/* boolean */
	int	max_block_size;		/* raw value as defined by MCL */
	int	max_fragment_size;	/* big files are fragmented into */
					/* fragments of this size */
} fec_codec_info_t;


class FluteFec {

public:

FluteFec::FluteFec(class flute_cb * flutecb);

FluteFec::~FluteFec();

/*
 * Updates the FEC available global variables depending on the available
 * FEC codecs.
 * Sets the FEC ratio for each possible FEC codec.
 * Retrieves the maximum block size for each possible FEC codec.
 */
void FindAvailableFEC (void);


/** 
 * RegisterFecCodec: lookup with mcl if the codec "codec" is available, 
 * if it is available then fill the codec_info fields max_block_size, and 
 * max_fragment size  else "codec_info->avalable" is set to 0 
 * 
 * @param codec the codec to register 
 * @param codec_info the fec_codec_info_t struct to fill 
 * @param codec_string a string of the mùane of the codec ( for possible error message)
 * @param max_fragment_size the max framgent size of the codec 
 * @param fec_ratio fec_ratio to use the codec with  
 */

void RegisterFecCodec(int codec,
					  fec_codec_info_t  *codec_info ,
					  char * codec_string, 
					  unsigned int max_fragment_size, 
					  float fec_ratio);
					  
	
/*
 * Choose the most appropriate FEC codec, depending on the file size
 * and the available FEC codecs.
 * @param file_size	file size
 * @return		FEC codec chosen
 */
fec_codec_info_t * ChooseFEC (int	file_size);


/* The possibles codecs */

fec_codec_info_t	NULL_FEC_info;
fec_codec_info_t	RSE_FEC_info;
fec_codec_info_t	LDGM_STAIRCASE_FEC_info;
fec_codec_info_t	LDGM_TRIANGLE_FEC_info;

private:

	class flute_cb *flutecb;

};


#endif
