/* $Id: mcl_lct_hdr.cpp,v 1.28 2005/01/11 13:12:29 roca Exp $ */
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


/*
 * private functions
 */
/* sending side */
static int lct_hdr_add_FTI_he	(mcl_cb *mclcb, char *ptr, int hlen,
		    		hdr_infos_t *hdr_infos);
static int lct_hdr_add_NONEWADU_he (mcl_cb *mclcb, char *ptr, int hlen,
			 	hdr_infos_t *hdr_infos);

static int lct_hdr_add_FDT_he	(mcl_cb *mclcb, char *ptr, int hlen,
				 hdr_infos_t *hdr_infos);
#ifdef METAOBJECT_USED
static int lct_hdr_add_MODT_he	(mcl_cb *mclcb, char *ptr, int hlen,
				 hdr_infos_t *hdr_infos);
#endif

/* receiving side */
static int lct_hdr_parse_he	(mcl_cb *mclcb, char *ptr, int hlen,
				hdr_infos_t *hdr_infos);


/****** sending side **********************************************************/

/**
 * Initializes the LCT header (fixed header plus common fields).
 * Assumes that the memory block has been allocated by the caller
 * who MUST ensure that it will be large enough to hold all the
 * possible fields!
 * Returns the size of the LCT header (bytes), -1 if error.
 */
int
lct_hdr_create (mcl_cb		*mclcb,
		fixed_lct_hdr_t	*lct_hdr,
		hdr_infos_t	*hdr_infos)
{
	INT32	hlen;	/* length of fixed+variable LCT hdr (bytes) */
	UINT32	word1;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_create:\n"))
	/*
	 * fixed size LCT header part
	 */
	*(UINT32*)lct_hdr = 0;	/* reset all flags */
	lct_hdr->version= LCT_VERSION;
	//lct_hdr->flag_c = 0;	/* no CC extension */
	//lct_hdr->reserved = 0;
	//lct_hdr->flag_h = 0;	/* no half word format by default */
	//lct_hdr->flag_s = 0;	/* no TSI field by default */
	//lct_hdr->flag_o = 0; 	/* no TOI field by default */
	//lct_hdr->flag_t = 0;	/* no SCT field by default */
	//lct_hdr->flag_r = 0;	/* no ERT field by default */
	lct_hdr->flag_a = (hdr_infos->close == true) ? 1 : 0;
	//lct_hdr->flag_b = 0;	/* no object close by default */
	lct_hdr->codepoint = hdr_infos->fec_encoding_id;
	/* NB: the FPI field is added by ALC if required ! */
	/*
	 * hlen (total header len) is different from lct_hdr->hdr_len
	 * (which only includes the header variable size part)
	 */
	hlen = sizeof(fixed_lct_hdr_t);	/* includes 32bit CCI field */
	ASSERT(!lct_hdr->flag_c);	/* no additional CCI extension */
	/*
	 * additional fields
	 */
	if (((hdr_infos->demux_label & 0xFFFF0000) == 0) &&
	    (hdr_infos->TOI_present && ((hdr_infos->idf_adu & 0xFFFF0000) == 0))) {
		/*
		 * use compact half word format since neither the TSI nor the
		 * TOI (when present) exceed 16 bits
		 */
		lct_hdr->flag_h = 1;	/* half word */
		word1 = ((UINT16)hdr_infos->demux_label << 16) |
			 (UINT16)hdr_infos->idf_adu;
		*(UINT32*)((char*)lct_hdr + hlen) = htonl(word1);
		hlen += 4;
	} else {
		/* add a 32 bit TSI (transport session idf) */
		lct_hdr->flag_s = 1;
		*(UINT32*)((char*)lct_hdr + hlen) =
						htonl(hdr_infos->demux_label);
		hlen += 4;
		if (hdr_infos->TOI_present) {
			/* add a 32 bit TOI (transport object idf) */
			lct_hdr->flag_o = 1;	/* 32 bits */
			*(UINT32*)((char*)lct_hdr + hlen) =
						htonl(hdr_infos->idf_adu);
			hlen += 4;
		}
	}
#if 0
	if (hdr_infos->demux_label) {
		/* add transport session idf (previously called demux_label) */
		lct_hdr->flag_s = 1;
		*(UINT32*)((char*)lct_hdr + hlen) =
						htonl(hdr_infos->demux_label);
		hlen += 4;
	}
	/* add a 32 bits TOI (tx object idf) */
	lct_hdr->flag_o = 1;	/* 32 bits */
	*(UINT32*)((char*)lct_hdr + hlen) = htonl(hdr_infos->idf_adu);
	hlen += 4;
#endif 
	/*
	 * add header extensions if any
	 */
	if (hdr_infos->FTI_present) {
		/* add an FTI extension */
		ASSERT(hdr_infos->adu_len > 0);
		hlen = lct_hdr_add_FTI_he(mclcb,(char*)lct_hdr, hlen, hdr_infos);
		if (hlen < 0) goto bad;
	}
	if (hdr_infos->FDT_present) {
		/* add an FDT extension */
		hlen = lct_hdr_add_FDT_he(mclcb,(char*)lct_hdr, hlen, hdr_infos);
		if (hlen < 0) goto bad;
	}
#ifdef METAOBJECT_USED
	if (hdr_infos->MODT_present) {
		/* add an MODT extension */
		hlen = lct_hdr_add_MODT_he(mclcb,(char*)lct_hdr, hlen, hdr_infos);
		if (hlen < 0) goto bad;
	}
#endif
	if (hdr_infos->NONEWADU_present) {
		ASSERT(hdr_infos->max_idf_adu >= mcl_iss);
		hlen = lct_hdr_add_NONEWADU_he(mclcb,(char*)lct_hdr, hlen, hdr_infos);
		if (hlen < 0) goto bad;
	}
	
	lct_hdr->hdr_len = hlen >> 2;		/* in 32-bit words */
	/*
	 * and finally swap the first 16 bits of header in network endian
	 */
	*(UINT16*)lct_hdr = htons(*(UINT16*)lct_hdr);
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_create: hlen=%d\n", hlen))
	return hlen;

bad:
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_create: ERROR\n"))
	return -1;
}
 

/**
 * Add a FTI header extension, using the provided hdr_infos, depending
 * on the FEC Encoding ID.
 *
 * With FEC Encoding IDs 0 (NO FEC) and 128 (SMALL BLOCK, LARGE  BLOCK
 * and EXPENDING):
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   HET = 64    |     HEL       |                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
 *  |                         Object Length (bytes)                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   FEC Instance ID             |    Encoding Symbol Length     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Max Source Block Length (max_k)                |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * With FEC Encoding IDs and 129 (SMALL SYSTEMATIC):
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   HET = 64    |     HEL       |                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
 *  |                         Object Length (bytes)                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   FEC Instance ID             |    Encoding Symbol Length     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |Max Source Block Length (max_k)| Max Nb of Enc. Symbols (max_n)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * With FEC Encoding ID 132 (for our LDGM/LDPC codec):
 * see the draft-peltotalo-rmt-bb-fec-xor-pcm-rs-XX.txt I-D...
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   HET = 64    |     HEL       |                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
 *  |                         Object Length (bytes)                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   FEC Instance ID             |    Encoding Symbol Length     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Max Source Block Length (max_k)                |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Max Nb of Enc. Symbols (max_n)                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  .                seed (optional, 0 or 32 bit long)              .
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this
 * header extension, or -1 if error.
 */
 
static int
lct_hdr_add_FTI_he (mcl_cb	*mclcb,
		    char	*ptr,
		    int		hlen,
		    hdr_infos_t	*hdr_infos)
{
	UINT32	word1 = 0;
	UINT32	word2 = 0;
	UINT32	word3 = 0;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_add_FTI_he:\n"))
	ASSERT(hdr_infos);
	ASSERT(hdr_infos->FTI_present);

	switch (hdr_infos->fec_encoding_id) {

#ifdef LDPC_FEC
	case FEC_ENCODING_ID_PCM_FEC:
		/* private format */
		word1 = (EXT_FTI << 24) | (6 << 16);
		*(UINT32*)(ptr + hlen) = htonl(word1);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->adu_len);
		hlen += 4;
		word2 = hdr_infos->fec_instance_id << 16 |
			(hdr_infos->symbol_len);
		*(UINT32*)(ptr + hlen) = htonl(word2);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->max_k);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->max_n);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->fec_key);
		hlen += 4;
		break;
#endif /* LDPC_FEC */

	case FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC:
		word1 = (EXT_FTI << 24) | (4 << 16);
		*(UINT32*)(ptr + hlen) = htonl(word1);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->adu_len);
		hlen += 4;
		word2 = (hdr_infos->fec_instance_id << 16) |
			(hdr_infos->symbol_len);
		*(UINT32*)(ptr + hlen) = htonl(word2);
		hlen += 4;
		word3 = (hdr_infos->max_k) << 16 | (hdr_infos->max_n);
		*(UINT32*)(ptr + hlen) = htonl(word3);
		hlen += 4;
		break;

	case FEC_ENCODING_ID_NO_FEC:
	case FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC:
		word1 = (EXT_FTI << 24) | (4 << 16);
		*(UINT32*)(ptr + hlen) = htonl(word1);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->adu_len);
		hlen += 4;
		word2 = (hdr_infos->fec_instance_id << 16) |
			(hdr_infos->symbol_len);
		*(UINT32*)(ptr + hlen) = htonl(word2);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->max_k);
		hlen += 4;
		break;

	default:
		ASSERT(0);
	}

	TRACELVL(5, (mcl_stdout, "<- lct_hdr_add_FTI_he: hlen=%d\n", hlen))
	return hlen;
}


/**
 * Add a header extension, using either the provided hdr_infos,
 * or by copying an externally supplied buffer containing the already
 * formatted extension header.
 * Updates the lct_hdr->hdr_len as required.
 *
 * This header extension is not LCT compliant (MCLv3 private extension)
 * and will not be used e.g. in FLUTE sessions.
 *
 * Compact version:
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |L|HET=NONEWADU |    HEL = 1    |      maximum TOI              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Long version:
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |L|HET=NONEWADU |    HEL = 2    |               0               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          maximum TOI                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this/these
 * extension header(s), or -1 if error.
 */
static int
lct_hdr_add_NONEWADU_he (mcl_cb	*mclcb,
			 char		*ptr,
			 int		hlen,
			 hdr_infos_t	*hdr_infos)
{
	UINT32	word1 = 0;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_add_NONEWADU_he:\n"))
	ASSERT(hdr_infos != NULL);
	ASSERT(hdr_infos->NONEWADU_present);
	ASSERT(hdr_infos->max_idf_adu >= mcl_iss);
	if ((hdr_infos->max_idf_adu & 0xFFFF0000) == 0) {
		/*
		 * use compact half word format since the maximum
		 * TOI does not exceed 16 bits
		 */
		word1 = (EXT_NONEWADU << 24) | (1 << 16) | (hdr_infos->max_idf_adu);
		*(UINT32*)(ptr + hlen) = htonl(word1);
		hlen += 4;
	} else {
		/*
		 * use long format since the maximum TOI exceeds 16 bits
		 */
		word1 = EXT_NONEWADU << 24 | (2 << 16);
		*(UINT32*)(ptr + hlen) = htonl(word1);
		hlen += 4;
		*(UINT32*)(ptr + hlen) = htonl(hdr_infos->max_idf_adu);
		hlen += 4;
	}
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_add_NONEWADU_he: max=%d\n",
		hdr_infos->max_idf_adu))
	return hlen;
}


/**
 * Add FDT instance header EXT_FDT.
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | HET=192       |   V   |        FDT instance ID                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this/these
 * extension header(s), or -1 if error.
 */
static int
lct_hdr_add_FDT_he (mcl_cb	*mclcb,
			 char		*ptr,
			 int		hlen,
			 hdr_infos_t	*hdr_infos)
{
	UINT32	word1 = 0;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_add_FDT_he:\n"))
	ASSERT(hdr_infos != NULL);
	ASSERT(mclcb->is_flute_compatible());
	ASSERT(hdr_infos->FDT_present);
	
	word1 = (EXT_FDT << 24) | (FDT_VERSION << 20) |
		(hdr_infos->FDT_instanceid & 0x000FFFFF) ;
	*(UINT32*)(ptr + hlen) = htonl(word1);
	hlen += 4;
	
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_add_FDT_he: FDT_instanceid=%d\n",
		hdr_infos->FDT_instanceid))
	return hlen;
}


#ifdef METAOBJECT_USED
/**
 * Add MODT instance header EXT_MODT.
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | HET=193       |   V   |        MODT instance ID                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this/these
 * extension header(s), or -1 if error.
 */
static int
lct_hdr_add_MODT_he (mcl_cb	*mclcb,
			 char		*ptr,
			 int		hlen,
			 hdr_infos_t	*hdr_infos)
{
	UINT32	word1 = 0;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_add_MODT_he:\n"))
	ASSERT(hdr_infos != NULL);
	ASSERT(hdr_infos->MODT_present);
	
	word1 = (EXT_MODT << 24) | (MODT_VERSION << 20) |
		(hdr_infos->FDT_instanceid & 0x000FFFFF);
	*(UINT32*)(ptr + hlen) = htonl(word1);
	hlen += 4;
	
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_add_MODT_he: max=%d\n",
		hdr_infos->max_idf_adu))
	return hlen;
}
#endif


/****** receiving side ********************************************************/


/**
 * Parses the LCT header of the received packet.
 * Assumes that hdr_infos is already reset.
 * Returns the size of the LCT header (bytes), -1 if error.
 */
int
lct_hdr_parse (mcl_cb		*mclcb,
	       fixed_lct_hdr_t	*lct_hdr,
	       hdr_infos_t	*hdr_infos)
{
	INT32	hlen;	/* length of fixed+variable LCT hdr (bytes) */
	UINT32	word1;

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_parse:\n"))
	/*
	 * first of all swap the first 16 bits of header in host endian
	 */
	*(UINT16*)lct_hdr = ntohs(*(UINT16*)lct_hdr);
	/* sanity checks */
	if ((lct_hdr->version != LCT_VERSION) || (lct_hdr->reserved != 0)) {
		PRINT_ERR((mcl_stderr,
		"lct_hdr_parse: ERROR, unknown version|non 0 reserved field\n"))
		goto bad_hdr;
	}
	/* check codepoint (in fact FEC Encoding ID) now */
	if ((lct_hdr->codepoint != FEC_ENCODING_ID_NO_FEC)
	    && (lct_hdr->codepoint != FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC)
	    && (lct_hdr->codepoint != FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC)
#ifdef LDPC_FEC
	    && (lct_hdr->codepoint != FEC_ENCODING_ID_PCM_FEC)
#endif
	   ) {
		PRINT_ERR((mcl_stderr,
			"lct_hdr_parse: ERROR, unsupported codepoint/FEC encoding ID (%d)\n",
			lct_hdr->codepoint))
		goto bad_hdr;
	}
	hdr_infos->fec_encoding_id = lct_hdr->codepoint;
	/* check header length in 32 bit words */
	if (lct_hdr->hdr_len < (1 + (1 + lct_hdr->flag_c) + lct_hdr->flag_h +
				lct_hdr->flag_s + lct_hdr->flag_o +
				lct_hdr->flag_t + lct_hdr->flag_r)) {
		PRINT_ERR((mcl_stderr,
			"lct_hdr_parse: ERROR, hdr_len %d too short\n",
			lct_hdr->hdr_len))
		goto bad_hdr;
	}
	/*
	 * header flags
	 */
	//hdr_infos->payload_id_present = lct_hdr->flag_p;/* FPI added by ALC */
	if (lct_hdr->flag_c != 0) {			/* CC extension */
		PRINT_ERR((mcl_stderr,
		"lct_hdr_parse: ERROR, CC of size > 32 bits not supported\n"))
		goto bad_hdr;
	}
	if (lct_hdr->flag_t | lct_hdr->flag_r) {	/* timing fields */
		PRINT_ERR((mcl_stderr,
			"lct_hdr_parse: ERROR, SCT/ERT not supported\n"))
		goto bad_hdr;
	}
	hdr_infos->close = (lct_hdr->flag_a == 1);	/* CLOSE session flag */
	if (lct_hdr->flag_b) {				/* CLOSE object flag */
		PRINT_ERR((mcl_stderr,
		"lct_hdr_parse: ERROR, flag B (close object) not supported, ignored!\n"))
		/*goto bad_hdr;*/
	}
	hlen = sizeof(fixed_lct_hdr_t);
	/*
	 * additional fields
	 */
	if (lct_hdr->flag_h) {
		/* half-word TSI/TOI format */
		if ((lct_hdr->flag_s != 0) || (lct_hdr->flag_o != 0)) {
			PRINT_ERR((mcl_stderr,
			"lct_hdr_parse: ERROR, half-word format with more than 16 bit TSI or TOI not supported\n"))
			goto bad_hdr;
		}
		/* 16 bit TSI (transport session idf) */
		word1 = ntohl(*(UINT32*)((char*)lct_hdr + hlen));
		hlen += 4;
		hdr_infos->demux_label = (UINT32)(word1 >> 16);
		/* 16 bit TOI (Transport Object Idf) */
		hdr_infos->TOI_present = true;
		hdr_infos->idf_adu = (word1 & 0x0000FFFF);
	} else {
		if (lct_hdr->flag_s) {
			/* 32 bit TSI (transport session Idf) */
			hdr_infos->demux_label =
				ntohl(*(UINT32*)((char*)lct_hdr + hlen));
			hlen += 4;
		} else {
			hdr_infos->demux_label = 0;
		}
		if (lct_hdr->flag_o > 0) {
			/* 32 bit TOI (Transport Object Idf) */
			hdr_infos->TOI_present = true;
			if (lct_hdr->flag_o == 1) {
				hdr_infos->idf_adu =
					ntohl(*(UINT32*)((char*)lct_hdr + hlen));
				hlen += 4;
			} else {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse: ERROR, TOI size > 32 bits not supported\n"))
				goto bad_hdr;
			}
		} else {
			hdr_infos->TOI_present = false;
			hdr_infos->idf_adu = 0;
		}
	}
	/*
	 * parse extension headers if any
	 */
	if (hlen < (int)(lct_hdr->hdr_len << 2)) {
		hlen = lct_hdr_parse_he(mclcb,(char*)lct_hdr, hlen, hdr_infos);
		if (hlen < 0) goto bad_hdr;
	}
	if (hlen != (int)(lct_hdr->hdr_len << 2)) {
		PRINT_ERR((mcl_stderr,
		"lct_hdr_parse: ERROR, bad hdr_len (announced %d, actual %d)\n",
		lct_hdr->hdr_len, (hlen>>2) - 2))
		goto bad_hdr;
	}
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_parse: demux_label=%d\n",
				hdr_infos->demux_label))
				
	return hlen;

bad_hdr:
	PRINT_ERR((mcl_stderr, "lct_hdr_parse: ERROR, bad header, dropped\n"))
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_parse: ERROR\n"))
	return -1;
}


/**
 * Parses the header extensions of the received packet.
 * Returns the new size of the header (bytes) with the HE field(s),
 * or <0 if error.
 */
static int
lct_hdr_parse_he (mcl_cb	*mclcb,
		  char		*ptr,
		  int		hlen,
		  hdr_infos_t	*hdr_infos)
{
	fixed_lct_hdr_t	*lct_hdr = (fixed_lct_hdr_t*)ptr;
	UINT32	word1;		/* first word of the HE */
	UINT32	word2;
	UINT32	word3;
	
	int		can_ignore = 0;	/* can be safely ignored bit */
	int		HET;
	int		HEL;
	int		eh_len;		/* hdr extension length (32bit words) */

	TRACELVL(5, (mcl_stdout, "-> lct_hdr_parse_he:\n"))
	/*
	 * go through all HE
	 */
	for (eh_len = lct_hdr->hdr_len - (hlen >> 2); eh_len > 0; ) {
		/* at least one word long! */
		word1 = ntohl(*(UINT32*)(ptr + hlen));
		hlen += 4;
		eh_len--;
		/*can_ignore = word1 >> 31;*/
		can_ignore = 0;		/* false for the present */
		HET = (word1 & 0xFF000000) >> 24;
		HEL = (word1 & 0x00FF0000) >> 16;	/* invalid if HET<64 */
		TRACELVL(5, (mcl_stdout,
			"   lct_hdr_parse_he: new HE, word=x%x, HET=%d, HEL=%d\n",
			word1, HET, HEL))
		switch (HET) {
		case EXT_FDT:
			if (mclcb->is_flute_compatible() == false) {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, EXT_FDT not supported if not in FLUTE compatibility mode\n"))
				goto bad;
			}
			if ((word1 & 0x00F00000) >> 20 != FDT_VERSION) {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, bad FDT version (expected %d, got %d)\n",
				FDT_VERSION, (word1 & 0x00F00000) >> 20))
				goto bad;
			}
			hdr_infos->FDT_instanceid= word1 & 0x000FFFFF;
			hdr_infos->FDT_present = true;
			break;

#ifdef METAOBJECT_USED
		case EXT_MODT:
			if ((word1 & 0x00F00000) >> 20 != MODT_VERSION) {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, bad MODT version (expected %d, got %d)\n",
				FDT_VERSION, (word1 & 0x00F00000) >> 20))
				goto bad;
			}
			hdr_infos->FDT_instanceid= word1 & 0x000FFFFF;
			hdr_infos->MODT_present = true;
			break;
#endif

		case EXT_FTI:
			/*
			 * the FTI format depends on the FEC Encoding ID,
			 * which has been checked before...
			 */
			/* assume the length (HEL) is ok, it will be checked
			 * just bellow... */			
			if ((word1 & 0X0000FFFF) != 0) {
				/* we don't support object length encoded with
				 * more than 32 bits */
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, upper 16 bits of 48 bit object length field non zero (not supported)\n"));
				goto bad;
			}
			hdr_infos->adu_len = ntohl(*(UINT32*)(ptr + hlen));
			hlen += 4;
			eh_len--;			 
			word2 = ntohl(*(UINT32*)(ptr + hlen));
			hdr_infos->fec_instance_id=(word2 & 0xFFFF0000)>> 16;
			
			switch (hdr_infos->fec_encoding_id) {
#ifdef LDPC_FEC
			case FEC_ENCODING_ID_PCM_FEC:
				if (HEL != 5 && HEL != 6) {
					PRINT_ERR((mcl_stderr,
					"lct_hdr_parse_he: ERROR, bad HEL (expected 5 or 6, got %d) for FTI of Encoding ID %d\n",
					HEL, hdr_infos->fec_encoding_id))
					goto bad;
				}
				hdr_infos->symbol_len = word2 & 0x0000FFFF;
				hlen += 4;
				eh_len--;
				hdr_infos->max_k = ntohl(*(UINT32*)(ptr + hlen));
				hlen += 4;
				eh_len--;
				hdr_infos->max_n = ntohl(*(UINT32*)(ptr + hlen));
				hlen += 4;
				eh_len--;
				if (HEL == 5) {
					hdr_infos->fec_key = 0;
				} else {
					hdr_infos->fec_key = ntohl(*(UINT32*)(ptr + hlen));
					hlen += 4;
					eh_len--;
				}
				break;
#endif /* LDPC_FEC */

			case FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC:
				if (HEL != 4) {
					PRINT_ERR((mcl_stderr,
					"lct_hdr_parse_he: ERROR, bad HEL (%d) for FTI of Encoding ID %d\n",
					HEL, hdr_infos->fec_encoding_id))
					goto bad;
				}
				hdr_infos->symbol_len=word2 & 0x0000FFFF;
				hlen += 4;
				eh_len--;
				word3 = ntohl(*(UINT32*)(ptr + hlen));
				hdr_infos->max_k = word3 >> 16;
				hdr_infos->max_n = word3 & 0x0000FFFF;
				hlen += 4;
				eh_len--;
				break;

			case FEC_ENCODING_ID_NO_FEC:
			case FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC:
				if (HEL != 4) {
					PRINT_ERR((mcl_stderr,
					"lct_hdr_parse_he: ERROR, bad HEL (%d) for FTI of Encoding ID %d\n",
					HEL, hdr_infos->fec_encoding_id))
					goto bad;
				}
				hdr_infos->symbol_len=word2 & 0x0000FFFF;
				hlen += 4;
				eh_len--;
				hdr_infos->max_k = ntohl(*(UINT32*)(ptr + hlen));
				hlen += 4;
				eh_len--;
				break;

			default:
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, unsupported FEC enconding ID (%d) in FTI hdr ext\n", hdr_infos->fec_encoding_id))
				goto bad;
			}
			hdr_infos->FTI_present = true;
			break;

		case EXT_NONEWADU:
			if (HEL == 1) {
				hdr_infos->max_idf_adu =
					(UINT32)(word1 & 0x0000FFFF);
			} else if (HEL == 2) {
				hdr_infos->max_idf_adu =
					ntohl(*(UINT32*)(ptr + hlen));
				hlen += 4;
			} else {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_he: ERROR, bad NONEWADU hdr ext\n"))
				goto bad;
			}
			eh_len--;
			hdr_infos->NONEWADU_present = true;
			break;

		case EXT_NOP:
			/* ignore */
			break;

		default:
			if (can_ignore) {
				/* first word already counted */
				hlen += (HET < 128) ? (HEL << 2) - 4 : 0;
				eh_len -= (HET < 64) ? HEL - 1 : 0;
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_HE: WARNING, unsupported HET %d (HEL=%d, word=x%x), ignored!\n",
				HET, HEL, word1))
				break;
			} else {
				PRINT_ERR((mcl_stderr,
				"lct_hdr_parse_HE: ERROR, unsupported HET %d (HEL=%d, word=x%x)\n",
				HET, HEL, word1))
				return -1;
			}
		}
	}
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_parse_he:\n"))
	return hlen;

bad:
	TRACELVL(5, (mcl_stdout, "<- lct_hdr_parse_he: ERROR\n"))
	return -1;
}

