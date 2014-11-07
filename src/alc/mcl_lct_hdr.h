/* $Id: mcl_lct_hdr.h,v 1.17 2005/01/11 13:12:29 roca Exp $ */
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

#ifndef MCL_LCT_HDR_H  /* { */
#define MCL_LCT_HDR_H


#define LCT_VERSION		1

#define CC_ID_RLC		1


/*
 * structure used for communications between the various header
 * creation/analysis functions and the rest of the code
 */
typedef struct hdr_infos {
	/* common fields */
	UINT32	demux_label;	/* demux label, also known as TSI */
	UINT32	fec_encoding_id;/* FEC encoding ID (RFC3452) */
	/* fields used for TOI */
	bool	TOI_present;	/* tx or rx: TOI present */
	UINT32	idf_adu;	/* ADU identifier (AKA TOI) */
	/* fields used by FPI */
	bool	FPI_present;	/* tx or rx: FEC payload ID present */
	UINT32	idf_block;	/* BLOCK identifier */
	UINT32 	idf_du;		/* DU identifier */
	bool 	is_fec;		/* true if DU has FEC content */
	/* fields used for File Delivery Table (FDT) with FLUTE */
	bool	FDT_present;	/* tx or rx: FDT present */
#ifdef METAOBJECT_USED
	bool	MODT_present;	/* tx or rx: MODT present */	
#endif
	UINT32	FDT_instanceid;	/* also used as MODT instanceid */
	/* fields used by FEC object Tx Info (FTI) */
	bool	FTI_present;	/* tx or rx: FTI present */
	UINT32 	adu_len;	/* ADU length in bytes */
	UINT32 	k;		/* _current_ src blk len in nb symbols*/
					/* used by some FPI */
	UINT32 	max_k;		/* max src blk len in nb of symbols */
					/* used by some FTI */
	UINT32	max_n;		/* max enc blk len in nb of symbols */
					/* used by some FTI */
	UINT16 	symbol_len;	/* full-sized symbol length in bytes */
	UINT16	fec_instance_id;/* FEC instance ID (RFC3452) */
#ifdef LDPC_FEC
	UINT32	fec_key;	/* LDGM/LDPC specific: random seed */
#endif
	/* fields used by NONEWADU */
	bool	NONEWADU_present;/* tx or rx: NONEWADU present */
	UINT32	max_idf_adu;	/* highest ADU identifier in session */
	/* fields used by CLOSE */
	bool	close;		/* CLOSE sig: boolean */
} hdr_infos_t;


/*
 * LCT header format
 */
typedef struct fixed_lct_hdr {
#ifdef  _BIT_FIELDS_LTOH
	UINT16	flag_b:1,
			flag_a:1,
			flag_r:1,
			flag_t:1,
			flag_h:1,
			flag_o:2,
			flag_s:1,
			reserved:2,
			flag_c:2,
			version:4;

	u_char		hdr_len;

	u_char		codepoint;
#else	/* the sames, in "natural" writting order... */			
	UINT16	version:4,	/* LCT version Number */
			flag_c:2,	/* congestion control flag */
			reserved:2,	/* unused; must be zero */
			flag_s:1,	/* transport session identifier flag */
			flag_o:2,	/* transport object identifier flag */
			flag_h:1,	/* half word flag */
			flag_t:1,	/* Sender Current Time (SCT) present */
			flag_r:1,	/* Expected Residual Time ERT present */
			flag_a:1,	/* close object flag */
			flag_b:1;	/* close session flag */

	u_char		hdr_len;	/* length of variable portion of LCT */
					/* header in units of 32-bit words */
					/* (starting from 3rd 32-bit word) */

	u_char		codepoint;	/* opaque idf used by payload decoder */
#endif
	UINT32	cci;		/* opaque congestion control header */
} fixed_lct_hdr_t;


/* types of signaling */
#define	EH_SIG		2	/* ALC/LCT signaling */
/* ... others... */


/*
 * Extension Headers (EH) types
 */
/* official EH defined by LCT */
#define EXT_NOP		0
#define	EXT_AUTH	1
/* official ALC-specific EH */
#define	EXT_FTI		64	/* FEC Object Transmission Information */
/* private EH (not in LCT/ALC drafts) */
#define EXT_NONEWADU	65	/* no new ADU will be submitted by the */
				/* application. Not used by FLUTE. */
/* FLUTE specific */
#define EXT_FDT		192	/* File Delivery Table Instance Header */

#define FDT_VERSION	1	/* FDT version defined in FLUTE spec */

#ifdef METAOBJECT_USED
/* private EH */
#define EXT_MODT	193	/* Meta-Object Description Table Instance hdr */

#define MODT_VERSION	1	/* private version number */
#endif

/*
 * Additional signaling types: must be coherent with the EXT_XXX types
 * Those are not EH...
 */
#define SIG_CLOSE	254	/* end of tx */


/*
 * Public function prototypes.
 */

/**
 * Create an LCT header.
 * @param mclcb
 * @param lct_hdr	pointer to the begining of the LCT header.
 * @param hdr_infos	pointer to the header info structure, that explains
 * 			how to create the LCT header.
 * @return
 */
extern int	lct_hdr_create	(class mcl_cb *mclcb, fixed_lct_hdr_t *lct_hdr,
				 hdr_infos_t *hdr_infos);

/**
 * Parse an LCT header.
 * @param mclcb
 * @param lct_hdr	pointer to the begining of the LCT header to analyze.
 * @param hdr_infos	pointer to the header info structure where is
 * 			stored the information contained in the LCT header.
 * @return
 */
extern int	lct_hdr_parse	(class mcl_cb *mclcb, fixed_lct_hdr_t *lct_hdr,
				 hdr_infos_t *hdr_infos);





#endif /* }  MCL_LCT_HDR_H */
