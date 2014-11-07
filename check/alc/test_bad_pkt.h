/* $Id: test_bad_pkt.h,v 1.1.1.1 2003/09/03 12:45:37 chneuman Exp $ */
/*
 * Copyright (c) 1999-2002 INRIA - Universite Paris 6 - All rights reserved
 * (main author: Vincent Roca - vincent.roca@inrialpes.fr)
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
/* author: Zainab KHALLOUF */


#if !defined(_H_TEST)
#define _H_TEST


#define CC_ID_RLC	0x55      /* Congestion control*/
#define FEC_ENCODING_NAME	0	/* for Reed-Solomon (TBC) */

/* EH defined by LCT */
#define EXT_NOP		0
#define	EXT_CCI		1
#define	EXT_TOI		2
#define	EXT_AUTH	3

/* EH defined by MCL */
#define EXT_FPI 4
#define EXT_FTI 5 


/*
 * print to stdout
 */
#define PRINT_OUT(a) { \
		fprintf a; \
		fflush(stdout); \
	}


#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif


/*
 * OS dependant code
 */


/* bit field order is compiler/OS dependant */
#if defined(LINUX) || defined(WIN32)
#define _BIT_FIELDS_LTOH
#undef _BIT_FIELDS_HTOL
#elif defined(SOLARIS)
#undef _BIT_FIELDS_LTOH
#define _BIT_FIELDS_HTOL        /* SunSparc is HtoL */
#endif


typedef struct lct_infos {
	u_int32_t	payload_id_present;	/* rx: block/du idf present */
	u_int32_t	idf_toi;	/* Obj. Id identifier */
	u_int32_t	idf_block;	/* BLOCK identifier */
	u_int32_t 	idf_du;		/* DU identifier */
	u_int32_t 	is_fec;		/* 1 if DU has FEC content */
	u_int32_t	demux_label;	/* demux label */
	u_int32_t	codepoint;	/* demux label */
	/* fields used by FTI (FEC object tx info) (previously NEWADU) */
	u_int32_t 	adu_len;	/* ADU length in bytes */
	u_int32_t 	block_len;	/* full-sized block length in bytes */
	/* fields used by NONEWADU */
	u_int32_t	max_idf_adu;	/* highest ADU identifier in session */
	/* fields used by CLOSE */
	u_int32_t	close;	        /* CLOSE sig: boolean */
        u_int32_t       cci_idf;        /* opaque congestion control header */
	u_int32_t       NOP_info;       /*Information must ignored by the receivers*/
} lct_infos_t;


typedef struct fixed_lct_hdr {
#ifdef  _BIT_FIELDS_LTOH
	u_int16_t	toi:10,
			flag_x:1,
			flag_t:1,
			reserved:1,
			flag_d:1,
			version:2;
	u_int8_t	hdr_len;
	u_int8_t	codepoint;

#else	/* the sames, in "natural" writting order... */			
	u_int16_t	version:2,	/* LCT version Number */
			flag_d:1,	/* Demux Label Present flag */
			reserved:1,	/* reserved; must be zero in the sender*/
			flag_t:1,	/* Transm Extension Info Present flag */
			flag_x:1,	/* Header-extension fields flag */
			toi:10;		/* Transport Object Identifier */

	u_int8_t	hdr_len;	/* length of variable portion of LCT */
					/* header in units of 32-bit words */
					/* (starting from 3rd 32-bit word) */
	u_int8_t	codepoint;	/* opaque idf used by the payload decoder */
#endif
	u_int32_t	cci;		/* opaque congestion control header */
} fixed_lct_hdr_t;


typedef enum ALL_THE_TESTS 
{
  /*LCT packet according to the Internet draft:draft-ietf-rmt-bb-lct-00.txt*/
  TEST_NORMAL_ACCORD_DRAFT=0 , 
  /*-----------------------------------------------------------------------------------*/
  /*LCT packet according to MCL*/  
  TEST_NORMAL_ACCORD_MCL=1,     
  /*------------------------------------------------------------------------------------*/
  /*Demux Label Present flag D = 1 and DL does not exist*/
  TEST_D_NOTDL=2,       
  /*-----------------------------------------------------------------------------------*/
   /*LCT version number(V) different of 0*/
  TEST_V=3      ,      
  /*-----------------------------------------------------------------------------------*/
  /*Code point(CP) with an erroneous value*/
  TEST_CP=4     ,       
  /*-----------------------------------------------------------------------------------*/
  /*Last header extension flag (L) = 0 in the last Header extension*/
  TEST_L=5      ,       
  /*-----------------------------------------------------------------------------------*/
  /*EXT not listed in the draft (in the draft there are :EXT_NOP ,EXT_CCI, EST_TOI, EXT_AUTH)*/
  TEST_NOT_EXT=6, 
  /*-----------------------------------------------------------------------------------*/
  /* Packets of the same session with different DL*/
  TEST_DIF_DL=7 ,      
  /*-----------------------------------------------------------------------------------*/
  /* Header extension present flag X = 1 and  Header Extensions does not exist */
  TEST_X_NOTHE=8,
  /*------------------------------------------------------------------------------------------*/
  /* LCT Header length (HDR_LEN) with an erroneous value*/
  TEST_HDR_LEN=9,    
  /*------------------------------------------------------------------------------------------*/
  /* Packets LCT of invalid SCT (time in the past) */
  TEST_HDR_PAST_SCT=10,  
  /*------------------------------------------------------------------------------------------*/
  /*Add Header extension FPI before the header extension TOI */
  TEST_REVERSE_EXT =11, 
  /*---------------------------------------------------------------------------*/
  /*Add a EXT_NOP header extension before the header extension TOI */
  TEST_NOP_EXT =12,  
  /*---------------------------------------------------------------------------*/
} ALL_THE_TESTS_t;

#undef EXTERN
#if defined(_C_TEST)
#         define EXTERN
#else
#         define EXTERN extern
#endif
 
/*-------------------------------------------------------------------------*/
EXTERN int lct_fixed_hdr_create (u_int16_t v                ,
                          	 u_int16_t d                ,
			   	 u_int16_t r                ,
			         u_int16_t t                ,
			         u_int16_t x                ,
		  	         lct_infos_t     *lct_infos ,
			         fixed_lct_hdr_t *lct_hdr   ,
			         int  hlen                  ,
			         u_int32_t sctTime          ,
			         u_int32_t ertTime          ,
			         int add_dl_flag            ,
			         int add_sct_ert_flag       ,
			         int add_he_toi_flag        ,
			         int add_he_fpi_flag        ,
			         int add_he_fti_flag        ,
			         int he_type                ,
			         int hel_he_toi             ,
			         int last_he_flag,
	                          int reverse_flag,
                                  int nop_flag);     
/*-------------------------------------------------------------------------*/
			
EXTERN int lct_hdr_add_DL(char *ptr, int hlen,u_int32_t demux_label);			         
/*-------------------------------------------------------------------------*/

EXTERN int lct_hdr_add_sct_ert (char *ptr, int hlen,u_int32_t timesct,u_int32_t timeert );
/*-------------------------------------------------------------------------*/

int lct_hdr_add_TOI_he	(char              *ptr      ,
			 u_int32_t          hlen     ,
			 u_int32_t          he_type  ,
			 u_int32_t          hel      ,
			 lct_infos_t *lct_infos,
			 u_int32_t is_last);
/*-------------------------------------------------------------------------*/

EXTERN int lct_hdr_generat (ALL_THE_TESTS_t choix_case, lct_infos_t *lct_infos,fixed_lct_hdr_t *lct_hdr);
/*-------------------------------------------------------------------------*/

EXTERN void DumpBuffer (char	*buf,
	    	        int	 len,
	                int	 to_dump);
/*-------------------------------------------------------------------------*/
EXTERN  int
lct_hdr_add_FPI_he (char	*ptr,
		    int		hlen,
		    lct_infos_t	*lct_infos,
		    u_int32_t is_last);
/*--------------------------------------------------------------------------*/
EXTERN  int
lct_hdr_add_FTI_he (char	*ptr,
		    int		hlen,
		    lct_infos_t	*lct_infos,
		    u_int32_t is_last);




/*--------------------------------------------------------------------------*/
EXTERN int lct_hdr_add_NOP_he	(char              *ptr,
			          u_int32_t  hlen,
			          lct_infos_t       *lct_infos,
			          u_int32_t is_last);

/*--------------------------------------------------------------------------*/
		    				
#endif
