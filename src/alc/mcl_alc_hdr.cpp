/* $Id: mcl_alc_hdr.cpp,v 1.9 2005/01/11 13:12:25 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
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


#include "mcl_includes.h"


static int alc_hdr_add_FPI_he	(mcl_cb *mclcb, char *ptr, int hlen,
				hdr_infos_t *hdr_infos);

static int alc_hdr_parse_FPI_he (mcl_cb *mclcb, char *ptr, int hlen, int plen,
				hdr_infos_t *hdr_infos);


/****** sending side **********************************************************/


/**
 * Initializes the ALC header.
 * => See header file for more informations.
 */
int
alc_hdr_create (mcl_cb		*mclcb,
		fixed_lct_hdr_t	*lct_hdr,
		hdr_infos_t	*hdr_infos)
{
	int	hlen;		/* length of the fixed+variable_len headers */

	TRACELVL(5, (mcl_stdout, "-> alc_hdr_create:\n"))
	if ((hlen = lct_hdr_create(mclcb, lct_hdr, hdr_infos)) < 0) {
		goto bad;
	}
	if (hdr_infos->FPI_present) {
		hlen = alc_hdr_add_FPI_he(mclcb, (char*)lct_hdr, hlen, hdr_infos);
	}
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_create:\n"))
	return hlen;

bad:
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_create: ERROR\n"))
	return -1;

}


/*
 * Add a FPI extension, using the provided hdr_infos.
 *
 * with FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC (128) and
 * FEC_ENCODING_ID_LDPC_FEC (140):
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      source block ID                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |F|                    encoding symbol ID                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * the F flag, if set, indicates that it is a FEC symbol 
 *
 *
 * with FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC (129):
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      source block ID                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      source block length      |     encoding symbol ID        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * with FEC_ENCODING_ID_NO_FEC:
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     source block ID           |      encoding symbol ID       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this
 * header extension, or -1 if error.
 */
static int
alc_hdr_add_FPI_he (mcl_cb	*mclcb,
		    char	*ptr,	/* write at ptr+hlen then update hlen */
		    int		hlen,
		    hdr_infos_t	*hdr_infos)
{
	UINT32	word = 0;

	TRACELVL(5, (mcl_stdout, "-> alc_hdr_add_FPI_he:\n"))
	ASSERT(hdr_infos);
	switch (hdr_infos->fec_encoding_id) {
	case FEC_ENCODING_ID_PCM_FEC:
	case FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC:
		/* 64 bit of FPI */
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->idf_block);
		hlen += 4;
		word = ((hdr_infos->is_fec ? 1 : 0) << 31) |
			(hdr_infos->idf_du & 0x7FFFFFFF);
		*(UINT32*)(ptr + hlen) = htonl(word);
		hlen += 4;
		break;

	case FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC:
		/* 64 bit of FPI */
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->idf_block);
		hlen += 4;
		word = ((hdr_infos->k) << 16) |
			(hdr_infos->idf_du & 0xFFFF);
		*(UINT32*)(ptr + hlen) = htonl(word);
		hlen += 4;
		break;

	case FEC_ENCODING_ID_NO_FEC:
		/* 32 bit of FPI */
		word = ((hdr_infos->idf_block & 0X0000FFFF) << 16) |
			(hdr_infos->idf_du & 0x0000FFFF);
		*(UINT32*)(ptr + hlen) = htonl(word);
		hlen += 4;
		break;

	default:
		ASSERT(0);
	}
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_add_FPI_he: hlen=%d\n", hlen))
	return hlen;
}


/****** receiving side ********************************************************/


/**
 * Parses the ALC header of the received packet.
 * => See header file for more informations.
 */
int
alc_hdr_parse (mcl_cb		*mclcb,
	       fixed_lct_hdr_t	*lct_hdr,
	       hdr_infos_t	*hdr_infos,
	       int		plen)	/* packet len (data+rlc+alc headers) */
{
	int	hlen;

	TRACELVL(5, (mcl_stdout, "-> alc_hdr_parse:\n"))
	if ((hlen = lct_hdr_parse(mclcb, lct_hdr, hdr_infos)) < 0) {
		goto bad_hdr;
	}
	if (plen > hlen) {
		if ((hlen = alc_hdr_parse_FPI_he(mclcb, (char*)lct_hdr, hlen,
						plen, hdr_infos)) < 0) {
			goto bad_hdr;
		} 
	}
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_parse: \n"))
	return hlen;

bad_hdr:
	PRINT_ERR((mcl_stderr, "alc_hdr_parse: ERROR, bad header, dropped\n"))
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_parse: error\n"))
	return -1;
}


static int
alc_hdr_parse_FPI_he (mcl_cb	*mclcb,
		    char	*ptr,	/* read at ptr+hlen */
		    int		hlen,
		    int		plen,
		    hdr_infos_t	*hdr_infos)
{
	UINT32	word;

	TRACELVL(5, (mcl_stdout, "-> alc_hdr_parse_FPI_he:\n"))
	switch (hdr_infos->fec_encoding_id) {
	case FEC_ENCODING_ID_PCM_FEC:
	case FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC:
		/* 64 bit of FPI */
		if (plen < hlen + 8) {
			TRACELVL(5, (mcl_stdout,
			"   alc_hdr_parse_FPI_he: ERROR, too short  with FEC Enc.ID 129/140 (expected %d, actual len %d)\n",
			hlen + 8, plen))
			return -1;
		}
		hdr_infos->idf_block = ntohl(*(int*)(ptr + hlen));
		hlen += 4;
		word = ntohl(*(int*)(ptr + hlen));
		hdr_infos->is_fec = (word & 0x80000000) ? true : false;
		hdr_infos->idf_du = (word & 0x7FFFFFFF);
		hlen += 4;
		break;

	case FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC:
		/* 64 bit of FPI */
		if (plen < hlen + 8) {
			TRACELVL(5, (mcl_stdout,
			"   alc_hdr_parse_FPI_he: ERROR, too short with FEC Enc.ID 129 (expected %d, actual len %d)\n",
			hlen + 8, plen))
			return -1;
		}
		hdr_infos->idf_block = ntohl(*(int*)(ptr + hlen));
		hlen += 4;
		word = ntohl(*(int*)(ptr + hlen));
		hdr_infos->k = (word & 0xFFFF0000) >> 16;
		hdr_infos->idf_du = (word & 0x0000FFFF);
		hdr_infos->is_fec = (hdr_infos->idf_du >= hdr_infos->k) ? true : false;
		hlen += 4;
		break;

	case FEC_ENCODING_ID_NO_FEC:
		/* 32 bit of FPI */
		if (plen < hlen + 4) {
			TRACELVL(5, (mcl_stdout,
			"   alc_hdr_parse_FPI_he: ERROR, too short with FEC Enc. ID 0 (expected %d, actual len %d)\n",
			hlen + 4, plen))
			return -1;
		}
		word = ntohl(*(int*)(ptr + hlen));
		hdr_infos->idf_block = (word >> 16);
		hdr_infos->is_fec = false;	/* no FEC here */
		hdr_infos->idf_du = (word & 0x0000FFFF);
		hlen += 4;
		break;

	default:
		ASSERT(0);	/* should have already been checked */
	}
	hdr_infos->FPI_present = true;
	TRACELVL(5, (mcl_stdout, "<- alc_hdr_parse_FPI_he: hlen=%d\n", hlen))
	return hlen;
}

