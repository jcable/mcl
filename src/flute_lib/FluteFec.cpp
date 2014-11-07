/* $Id: FluteFec.cpp,v 1.2 2005/05/12 16:03:35 moi Exp $ */
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

FluteFec::FluteFec(class flute_cb * flutecb) {

	ASSERT(flutecb->mode == SEND);
	this->flutecb = flutecb;
	FindAvailableFEC();

}

FluteFec::~FluteFec() {
}

				  
/*
 * Updates the FEC available global variables depending on the available
 * FEC codecs.
 * Sets the FEC ratio for each possible FEC codec.
 * Retrieves the maximum block size for each possible FEC codec.
 */
void FluteFec::FindAvailableFEC (void)
{
  ASSERT(flutecb->fec_ratio >= 1.0);
  /*
   * NULL_FEC codec first (should always be supported)
   */
  RegisterFecCodec( MCL_FEC_SCHEME_NULL,
					&NULL_FEC_info,"NULL",
					NULL_FEC_MAX_FRAGMENT_SIZE,flutecb->fec_ratio);
	  
  /*
   * now let's see the other FEC codecs
   */
  if (flutecb->fec_ratio == 1.0) {
	/* user wants no FEC */
	RSE_FEC_info.available = 0;
	LDGM_STAIRCASE_FEC_info.available = 0;
	LDGM_TRIANGLE_FEC_info.available = 0;
  } else {
	/* user wants some FEC */
	/*
	 * RSE
	 */
	  
	RegisterFecCodec( MCL_FEC_SCHEME_RSE_129_0,
					  &RSE_FEC_info,"RSE",
					  RSE_MAX_FRAGMENT_SIZE,flutecb->fec_ratio);
		
	/* tests MCL blocking algo: define a large frag size */
	//RSE_FEC_info.max_fragment_size = 512*1024;
	  
#if defined(ALC)
	/*
	 * LDGM STAIRCASE
	 */
	RegisterFecCodec(MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0,
					 &LDGM_STAIRCASE_FEC_info,
					 "LDGM STAIRCASE",
					 LDGM_MAX_FRAGMENT_SIZE,
					 flutecb->fec_ratio );
	/*
	 * LDGM TRIANGLE
	 */
	RegisterFecCodec(MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1 ,
					 &LDGM_TRIANGLE_FEC_info ,
					 "LDGM TRIANGLE ", 
					 LDGM_MAX_FRAGMENT_SIZE,
					 flutecb->fec_ratio );
	
	
#elif defined(NORM)
	LDGM_STAIRCASE_FEC_info.available = 0;
	LDGM_TRIANGLE_FEC_info.available = 0;
#endif /* RM_PROTOCOL */
	// Remove following tow lines comment to test with RSE
	//LDGM_STAIRCASE_FEC_info.available = 0;
	//LDGM_TRIANGLE_FEC_info.available = 0;
	if (!RSE_FEC_info.available && !LDGM_STAIRCASE_FEC_info.available && !LDGM_TRIANGLE_FEC_info.available) {
	  EXIT(("Flute: ERROR, at least one of RSE or LDGM codecs must be available\n"))
		}
  }
}


/*
 * Choose the most appropriate FEC codec, depending on the file size
 * and the available FEC codecs.
 * @param file_size	file size
 * @return		FEC codec chosen
 */
fec_codec_info_t *
FluteFec::ChooseFEC (int	file_size)
{
	int		codec;
	fec_codec_info_t *codec_info;

	if (flutecb->fec_ratio == 1.0) {
		ASSERT(NULL_FEC_info.available);
		codec = MCL_FEC_SCHEME_NULL;
		codec_info = &NULL_FEC_info;
		goto codec_found;
	}
	/*
	 * With ALC, use LDGM first for big files and RSE for
	 * small files, and if the default choice is not available,
	 * use the other one.
	 */
	if (file_size >= RSE_LDGM_FILE_SIZE_THRESHOLD) {
		if (flutecb->fec_ratio >= 2.5 && LDGM_STAIRCASE_FEC_info.available) {
			/*
			 * Use LDGM Staircase with large FEC expansion ratios.
			 */
			codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
			codec_info = &LDGM_STAIRCASE_FEC_info;
		} else if (LDGM_TRIANGLE_FEC_info.available) {
			/*
			 * Use LDGM Triangle with small FEC expansion ratios.
			 */
			codec = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
			codec_info = &LDGM_TRIANGLE_FEC_info;
		} else {
			ASSERT(RSE_FEC_info.available)
			codec = MCL_FEC_SCHEME_RSE_129_0;
			codec_info = &RSE_FEC_info;
		}
	} else {
		if (RSE_FEC_info.available) {
			codec = MCL_FEC_SCHEME_RSE_129_0;
			codec_info = &RSE_FEC_info;
		} else {
			ASSERT(LDGM_STAIRCASE_FEC_info.available)
			codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
			codec_info = &LDGM_STAIRCASE_FEC_info;
		}
	}

codec_found:
	/* set the codec now */
	if (mcl_ctl(flutecb->id, MCL_OPT_SET_FEC_CODE, (void*)&codec, sizeof(codec))) {
		EXIT(("Flute: ERROR, ctl for MCL_OPT_SET_FEC_CODE (%d) failed\n", codec))
	}
	return codec_info;
}


#if 0
/*
 * Determines the optimum file segmentation, and the appropriate FEC codec
 * to use.
 * @param
 * @return
 */
int
FluteFec::FileSegmentation
{
}
#endif



void FluteFec::RegisterFecCodec(int codec,
					  fec_codec_info_t  *codec_info ,
					  char * codec_string, 
					  unsigned int max_fragment_size, 
					  float fec_ratio){
  
  int max_sz;
  if (mcl_ctl(flutecb->id, MCL_OPT_SET_FEC_CODE, (void*)&codec,
			  sizeof(codec))) {
	codec_info->available = 0;
  }else {
	codec_info->available = 1;
	if (mcl_ctl(flutecb->id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio,
				sizeof(fec_ratio)))
	  	EXIT(("Flute: ERROR, mcl_ctl failed for FEC_RATIO %f for %s codec\n", fec_ratio,codec_string));
	/*
	 * determine the max block size, as defined by MCL,
	 * and the associated max file fragment size, used
	 * by FCAST.
	 */
	if (mcl_ctl(flutecb->id,
				MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC,
				(void*)&max_sz, sizeof(max_sz))) {
	  EXIT(("Flute: ERROR, mcl_ctl failed for MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC for %s codec\n",codec_string));
	}
	codec_info->max_block_size = max_sz;
	codec_info->max_fragment_size =  min(LDGM_MAX_FRAGMENT_SIZE,  max_sz);
  }
}



