/* $Id: test_bad_pkt.c,v 1.1.1.1 2003/09/03 12:45:37 chneuman Exp $ */
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

#define _C_TEST

#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "test_bad_pkt.h"

#define MAXLEN 1024
#define TTL 1

/*-------------------------------------------------------------------------*/
/*
 * buffer dump
 */
void
DumpBuffer (char	*buf,
	    int		len,
	    int		to_dump)	/* number of u32 words to dump */
{
	int	i, j;
	int	*ptr;

	i = min(to_dump, (len >> 2));
	j = 0;
	PRINT_OUT((stdout, "\tDUMP %d first bytes... (len=%d/dump=%d)\n\t",
	           i * sizeof(int),
		len, to_dump))
	for (ptr = (int*)buf; i > 0; i--, ptr++) {
		/* convert to network format to be sure of byte order */
		PRINT_OUT((stdout, "%08x ",htonl(*ptr)));
                 
		if (++j == 8) {
			j = 0;
			PRINT_OUT((stdout, "\n\t"))
		}
	}
	PRINT_OUT((stdout, "\n"))
}
/*-------------------------------------------------------------------------*/
int lct_fixed_hdr_create 
         (u_int16_t v ,/* LCT version Number */
          u_int16_t d ,/* Demux Label Present flag */
	  u_int16_t r ,/* reserved; must be zero in the sender*/
	  u_int16_t t ,/* Transm Extension Info Present flag */
	  u_int16_t x ,/* Header-extension fields flag */ 

	  lct_infos_t     *lct_infos ,
	  fixed_lct_hdr_t *lct_hdr   ,
	  int  hlen ,  /*initial value for HDR_LEN, necessair
			 for the test TEST_HDR_LEN*/


	  u_int32_t sctTime,/*Sender current time in ms*/   
	  u_int32_t ertTime,/*ERT in ms*/
	  int add_dl_flag      ,/*to add a DL field*/ 
	  int add_sct_ert_flag ,/*to add a SCT,ERT  fields*/ 


	  int add_he_toi_flag ,/*to add a header extension TOI */

	  int add_he_fpi_flag ,/*to add a header extension FPI (forword
			        *error correction payload identifier)*/

	  int add_he_fti_flag ,/*to add a header extension FTI(forward
			        *error correction object transmission
				*identifier)*/

	  int he_type         ,/*to add header extension witch type not 
			         listed in the draft necessair for the 
				 test TEST_NOT_EXT*/

	  int hel_he_toi      ,/*header extension length*/
	  int last_he_flag    ,/*L in the last header extension must be 1
			               *last_he_flag = 1 <=> L=1 in
				       *all header extension 
				       *last_he_flag = 0 <=> L=0 in 
				       *all header extension 
				       *last_he_flag = 2 <=> L=1 in 
				       *the last header extension */


	  int reverse_flag    , /*to add header extension FPI before 
			          the header extension TOI*/			       

	   int nop_flag)        /*Add Header extension NOP before the header 
	                          extension TOI,Suivant the draft the receiver 
				  must ignore the information in this field*/				 
						       
						       
						       
{
   u_int32_t *last_ext = NULL;
   int L=0 ;/*Flag in the last header extension*/

   lct_hdr->version = v &0x3;
   lct_hdr->flag_d  = d & 0x1;	
   lct_hdr->reserved= r & 0x1;
   lct_hdr->flag_t  = t & 0x1;
   lct_hdr->flag_x  = x & 0x1;
    lct_hdr->toi	  = ((lct_infos->idf_toi)  & 0x3ff);
   lct_hdr->codepoint = ((lct_infos->codepoint)& 0xff);
   lct_hdr->cci       = htonl((lct_infos->cci_idf)  & 0xffffffff);
   /*--------------------------------------------------------*/
   /*length of fixed and variable part of lct header*/
   hlen = hlen  + sizeof(fixed_lct_hdr_t);
   if ((lct_infos->demux_label)&& (add_dl_flag)) {
	   /* add a demux label */
	  hlen=lct_hdr_add_DL((char*)lct_hdr,hlen,lct_infos->demux_label);
	  if (hlen < 0) goto bad;
   }
   /*--------------------------------------------------------*/
   if (add_sct_ert_flag){
	   hlen =lct_hdr_add_sct_ert ((char*)lct_hdr,hlen,sctTime,ertTime);
	   if (hlen < 0) goto bad;

   }
   if (last_he_flag == 2) L=1;

   /*--------------------------------------------------------*/

   if (nop_flag){
    /* add an NOP EXT  */
    last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* to add L bit */
    hlen=lct_hdr_add_NOP_he((char*)lct_hdr, hlen, lct_infos,L);
    if (hlen < 0) goto bad;
   }

   if (reverse_flag)
   {
	   if ((lct_infos->payload_id_present)&&(add_he_fpi_flag)) {
	   /* add an FPI (FEC payload idf) extension (required if data) */
	   last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* to add L bit */
	   hlen = lct_hdr_add_FPI_he((char*)lct_hdr, hlen, lct_infos,L);
	   if (hlen < 0) goto bad;
	 }

 	   if (((lct_infos->idf_toi)> 1023) && (add_he_toi_flag)){
	   last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* To add L bit */
   	   hlen     = lct_hdr_add_TOI_he((char*)lct_hdr,hlen,he_type,
	   hel_he_toi,lct_infos,L);
	   if (hlen < 0) goto bad;
	   lct_hdr->toi = 0;
	   }

   }
   else/*Normal case*/
   {
	   if (((lct_infos->idf_toi)> 1023) && (add_he_toi_flag)){
	   last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* To add L bit */

	   hlen     = lct_hdr_add_TOI_he((char*)lct_hdr,hlen,he_type,
	   hel_he_toi,lct_infos,L);

	   if (hlen < 0) goto bad;
	   lct_hdr->toi = 0;
	   }

	   if ((lct_infos->payload_id_present)&&(add_he_fpi_flag)) {
	   /* add an FPI (FEC payload idf) extension (required if data) */
	   last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* to add L bit */
	   hlen = lct_hdr_add_FPI_he((char*)lct_hdr, hlen, lct_infos,L);
	   if (hlen < 0) goto bad;
           }
   }//reverse_flag

	   if ((lct_infos->adu_len > 0)&&(add_he_fti_flag)) {
	   /* add an FTI (FEC objet tx idf) extension */
	   last_ext = (u_int32_t*)((char*)lct_hdr+hlen);/* to add L bit */
	   hlen = lct_hdr_add_FTI_he((char*)lct_hdr, hlen, lct_infos,L);
	   if (hlen < 0) goto bad;
   }

   /*--------------------------------------------------------*/
   if (last_ext){lct_hdr->flag_x= 1;}
   if ((last_he_flag==2)&&(last_ext)) /*L=1 in the last header extension */
	   {
	   u_int32_t add_L;
	   add_L = ntohl(*last_ext);
	   add_L |= 0x80000000;
	   *last_ext = htonl(add_L);}

   lct_hdr->hdr_len =(hlen - 8) >> 2;	/*in 32-bit words */

   /*
    * and finally swap the first 16 bits of header in network endian
    */
    *(u_int16_t*)lct_hdr = htons(*(u_int16_t*)lct_hdr);
   return  hlen;
bad:
   return -1;
	
}	
 /*--------------------------------------------------------*/
int lct_hdr_add_DL(char *ptr, int hlen,u_int32_t demux_label)
{
  /* add a demux label */
     fixed_lct_hdr_t *lct_hdr = (fixed_lct_hdr_t*)ptr;
     *(u_int32_t*)((char*)lct_hdr + hlen) = htonl(demux_label);
     hlen += 4;
     return hlen;
}/*lct_hdr_add_DL_he*/

/*--------------------------------------------------------*/
int lct_hdr_add_sct_ert (char *ptr, int hlen,u_int32_t timesct,u_int32_t timeert)
{
  fixed_lct_hdr_t *lct_hdr = (fixed_lct_hdr_t*)ptr;
  *(u_int32_t*)((char*)lct_hdr + hlen) = htonl(timesct);
   hlen += 4;
  *(u_int32_t*)((char*)lct_hdr + hlen) = htonl(timeert);
   hlen += 4; 
   return hlen;
}/*lct_hdr_add_SCT_and_ERT*/

 /*----------------------------------------------------------------------
   * Add a EXT_NOP  header extension, using the provided lct_infos.
   * Updates the lct_hdr as required.
   *
   * 0                   1                   2                   3
   * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |L| HET=EXT_NOP |    HEL = 2    |               0               |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |          Information must be ignored by receivers             |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *
   * Returns the new size of the header (bytes) with this
   * header extension, or -1 if error.
   *
 *------------------------------------------------------------------------*/
int lct_hdr_add_NOP_he	(char              *ptr      ,
			 u_int32_t          hlen     ,
			 lct_infos_t       *lct_infos,
			 u_int32_t is_last)
{
	u_int32_t	word1 = 0;
	word1 = (is_last << 31)|(EXT_NOP << 24)|(2 << 16);
	*(u_int32_t*)(ptr + hlen) = htonl(word1);
	hlen += 4;
	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->NOP_info);
	hlen += 4;
	return hlen;
}

/*------------------------------------------------------------------------
   * Add a TOI header extension, using the provided lct_infos.
   * Updates the lct_hdr as required.
   *
   * 0                   1                   2                   3
   * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |L| HET=EXT_TOI |    HEL = 2    |               0               |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                        object idf                             |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *
   * Returns the new size of the header (bytes) with this
   * header extension, or -1 if error.
   *
   * It must that HEL <=255
 *------------------------------------------------------------------------*/
int lct_hdr_add_TOI_he	(char              *ptr      ,
			 u_int32_t          hlen     ,
			 u_int32_t          he_type  ,
			 u_int32_t          hel      ,
			 lct_infos_t       *lct_infos,
			 u_int32_t is_last)
{
	u_int32_t	word1 = 0;
	
	word1 = (is_last << 31)|(EXT_TOI << 24)|(2 << 16);
	*(u_int32_t*)(ptr + hlen) = htonl(word1);
	hlen += 4;
	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->idf_toi);
	hlen += 4;
	return hlen;
}
/*--------------------------------------------------------------------------*/
/*
 * Add a FPI header extension, using the provided lct_infos.
 * Updates the lct_hdr as required.
 *
 * LCT_HDR_OPT_FOR_SPACE version (not LCT compliant):
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |L| HET=EXT_FPI |    HEL = 2    |F|       symbol idf            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    block  idf                                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * the F flag indicates that it is a FEC symbol 
 *
 * if we do not want to use LCT_HDR_OPT_FOR_SPACE,this version suive the internet
 * draft
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |L| HET=EXT_FPI |    HEL = 3    |F|                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    symbol idf                                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    block  idf                                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Returns the new size of the header (bytes) with this
 * header extension, or -1 if error.
 *
 */
 int
lct_hdr_add_FPI_he (char	*ptr,
		    int		hlen,
		    lct_infos_t	*lct_infos,
		    u_int32_t is_last)
{
	u_int32_t	word1 = 0;
		word1 = (is_last << 31)|(EXT_FPI << 24) | (3 << 16) |((lct_infos->is_fec ? 1 : 0) << 15);
	*(u_int32_t*)(ptr + hlen) = htonl(word1);
	hlen += 4;
		
	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->idf_du & 0xffffffff);
	hlen += 4;

	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->idf_block & 0xffffffff);
	hlen += 4;
	return hlen;
}
/*-----------------------------------------------------------------------*/
/*
 * Add a FTI header extension, using the provided lct_infos.
 * Updates the lct_hdr as required.
 *
 * LCT_HDR_OPT_FOR_SPACE version (not LCT compliant):
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |L| HET=EXT_FTI |    HEL = 3    |     FEC encoding name         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  object length in bytes                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   block length in bytes                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Returns the new size of the header (bytes) with this
 * header extension, or -1 if error.
 */
 int
lct_hdr_add_FTI_he (char	*ptr,
		    int		hlen,
		    lct_infos_t	*lct_infos,
		    u_int32_t is_last)
{
	u_int32_t	word1 = 0;
	
	/* skip the FEC encoding name and use space for HET/HEL instead */
	word1 = (is_last << 31)|(EXT_FTI << 24) | (3 << 16) | 
	        (FEC_ENCODING_NAME & 0xffff);
	*(u_int32_t*)(ptr + hlen) = htonl(word1);
	hlen += 4;
	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->adu_len);
	hlen += 4;
	*(u_int32_t*)(ptr + hlen) = htonl(lct_infos->block_len);
	hlen += 4;
	
	return hlen;
}

/*-----------------------------------------------------------------------*/
int lct_hdr_generat 
   (ALL_THE_TESTS_t choix_case,
    lct_infos_t *lct_infos,
    fixed_lct_hdr_t *lct_hdr)
{
 u_int32_t ertTime       = 10000;
 u_int32_t  Past_sctTime = 10000;
 int hln         ;
 int dl          ;
 int hlen                  ;	
 int cp                    ;
 int other_ext             ;
 switch (choix_case) {
 case TEST_NORMAL_ACCORD_DRAFT:
 printf ("LCT packet according to the Internet draft:draft-ietf-rmt-bb-lct-00.txt\n");
 hln = lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                            lct_infos ,
			    lct_hdr   ,
				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/ 
				       0 	 , /*add_he_fpi_flag*/
				       0	 , /*add_he_fti_flag*/
			               EXT_TOI   ,     
				       2         , /*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 
 break;
case TEST_NORMAL_ACCORD_MCL:
printf ("LCT packet according to MCL\n");
hln = lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                            lct_infos ,
			    lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/ 
				       1  	 , /*add_he_fpi_flag*/
				       0 	 , /*add_he_fti_flag*/
			               EXT_TOI   ,     
				       2         , /*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 
 break;
 case TEST_D_NOTDL:
 printf("Demux Label Present flag D = 1 and DL does not exist \n");
      	 hln = 
		 lct_fixed_hdr_create (0,1,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag */
				       0		, /*add_he_fpi_flag*/
				       0		, /*add_he_fti_flag*/
			               EXT_TOI   ,     
				       2         ,/*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

 break;
 case TEST_V:
 printf("LCT version number(V) different of 0 \n");
      	 /*I will do this later v = 1,2,3*/
	 hln = 
		 lct_fixed_hdr_create (1,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/ 
				       0         ,
				       0         ,
			               EXT_TOI   ,     
				       2         ,/*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

 break;
 case TEST_CP:
 printf("Code point(CP) with an erroneous value\n");

		 cp = random ()/20000000;
	         lct_infos->codepoint = cp;
		 hln = 
		 lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/ 
				       0         ,
				       0         ,
			               EXT_TOI   ,     
				       2         ,     /*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

 break;
 case TEST_L:
		 lct_infos->idf_toi = 1024; /*we must use header exention*/
		 printf("Last header extension flag (L) = 0 in the last Header extension\n");
                 hln = 
		 lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag */
			               0,
				       0,
				       EXT_TOI   ,     
				       2         ,     /*hel_he_toi */
			               0         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

 break;
 case TEST_NOT_EXT:
		 printf("EXT not listed in the draft(in the draft there are :EXT_NOP ,EXT_CCI, EST_TOI, EXT_AUTH) \n");
                 other_ext = random ()/20000000;
		 lct_infos->idf_toi = 1024; /*we must use header exention*/
		 hln = 
		 lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,

				       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/ 
				       0         ,
				       0         ,
			               other_ext ,     
				       2         , /*hel_he_toi */
			               2         , /*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

 break;
 case TEST_DIF_DL:
	 printf(" Packets of the same session with different DL\n");
      	 dl = random ()/20000000;
	 lct_infos->demux_label = dl;
	 hln = 
	 lct_fixed_hdr_create (0,1,0,0,0 , /*V,D,r,T,X*/
                         	       lct_infos ,
			               lct_hdr   ,
			 	       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               1         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag */
				       0         ,
				       0	 ,
			               EXT_TOI   ,     
				       2         ,/*hel_he_toi */
			               2         ,/*last_he*/
				       0         ,/*reverse_flag*/
				       0);         /*NOP EXT*/ 
  break;
  case TEST_X_NOTHE:
       printf(" Header extension present flag X = 1 and  Header Extensions does not exist\n");
       lct_infos->idf_toi = 1024; /*we must use header exention*/
		     
	hln = 
	lct_fixed_hdr_create (0,0,0,0,1, /*V,D,r,T,X*/
                         	      lct_infos ,
			              lct_hdr   ,
			 	       0         , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               0         , /*add_he_toi_flag*/ 
				       0	 ,
				       0	 ,
			               EXT_TOI   ,     
				       2         ,     /*hel_he_toi */
			               2         ,/*last_he*/
				       0         , /*reverse_flag*/
				       0);         /*NOP EXT*/ 

  break;
  case TEST_HDR_LEN:
	 printf("LCT Header length (HDR_LEN) with an erroneous value \n");

	 hlen  = random ()/20000000;
	 hln = 
	 lct_fixed_hdr_create (0,0,0,0,0, /*V,D,r,T,X*/
                         	     lct_infos ,
			             lct_hdr,

				       hlen      , /*hlen*/
			               0         , /*sctTime */ 
			               0         , /*ertTime */  
			               0         , /*add_dl_flag*/   
			               0         , /*add_sct_flag*/
			               1         , /*add_he_toi_flag*/
				       0	 ,
				       0	 ,
			               EXT_TOI   ,     
				       2         ,/*hel_he_toi */
			               2         ,/*last_he*/
				       0         ,/*reverse_flag*/
				       0);         /*NOP EXT*/ 
 break;
 /*-----------------------------------------------------------------------*/
  case TEST_HDR_PAST_SCT:
  printf("Packets LCT of invalid SCT (time in the past)\n");
         
	   hln = 
		 lct_fixed_hdr_create (0,0,0,1,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,
			 	 0         ,   /*hlen*/
			         Past_sctTime ,/*sctTime */ 
			         ertTime   , /*ertTime */  
			         0         , /*add_dl_flag*/   
			         1         , /*add_sct_flag*/
			         1         , /*add_he_toi_flag */
				 0	   ,
				 0	   ,
			         EXT_TOI   ,     
				 2         , /*hel_he_toi */
			         2         , /*last_he*/
				 0         , /*reverse_flag*/
				 0);         /*NOP EXT*/ 
 break;
 /*-----------------------------------------------------------------------*/
 case TEST_REVERSE_EXT:
  printf("Add header extension FPI before the header extension TOI \n");
      	 lct_infos->idf_toi = 1024; /*we must use header exention*/
	 hln = 
		 lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,
			 	 0         ,   /*hlen*/
			         Past_sctTime, /*sctTime */ 
			         ertTime   ,/*ertTime */  
			         0         ,/*add_dl_flag*/   
			         0         ,/*add_sct_flag*/
			         1         ,/*add_he_toi_flag */
				 1	   ,
				 0	   ,
			         EXT_TOI   ,     
				 2         ,/*hel_he_toi */
			         2         ,/*last_he*/
				 1         , /*reverse_flag*/
				 0);         /*NOP EXT*/ 
 break;
 /*-----------------------------------------------------------------------*/
 case TEST_NOP_EXT:
  printf("Add a EXT_NOP header extension before the header extension TOI  \n");
      	 lct_infos->idf_toi = 1024; /*we must use header exention*/
	 hln = 
		 lct_fixed_hdr_create (0,0,0,0,0 , /*V,D,r,T,X*/
                         	 lct_infos ,
			         lct_hdr   ,
			 	 0         ,   /*hlen*/
			         Past_sctTime, /*sctTime */ 
			         ertTime   ,/*ertTime */  
			         0         ,/*add_dl_flag*/   
			         0         ,/*add_sct_flag*/
			         1         ,/*add_he_toi_flag */
				 0	   ,
				 0	   ,
			         EXT_TOI   ,     
				 2         ,/*hel_he_toi */
			         2         ,/*last_he*/
				 0         , /*reverse_flag*/
				 1);         /*NOP EXT*/ 
 break;
 /*-----------------------------------------------------------------------*/
 default:
	 printf("ERROR: unknown test %d", choix_case);
	 exit(1);

 }/*switch*/

hln = hln >> 2; /*in 32 bits word*/ 	
return hln ;

}/*lct_hdr_generat*/
/*
 * send a LCT packet according to the test number, Returns 0 if OK, < 0 otherwise.
 */
int main(int argc, char* argv[])
{
    const u_char yes = 1;      /* Used with SO_REUSEADDR */
    int send_s;                /* Sockets for sending and receiving*/
    struct sockaddr_in mcast_group;
    char IOBuf [MAXLEN];
 
    lct_infos_t          lct_infos_m;
    fixed_lct_hdr_t	*lct_hdr_m  ;  /* fixed size LCT header */
    int		hlen   ;		
    int choix ; 
    u_char ttl;
    if ((argc <4)||(argc >5)) {
        fprintf(stderr, "Usage: %s mcast_group port num_test [ttl]\n", argv[0]);
        exit(1);
    }

    memset(IOBuf, 0, sizeof(IOBuf));
    memset(&mcast_group, 0, sizeof(mcast_group));
    mcast_group.sin_family      = AF_INET;
    mcast_group.sin_port        = htons((unsigned short int)strtol(argv[2], NULL, 0));
    mcast_group.sin_addr.s_addr = inet_addr(argv[1]);
    choix =(int)strtol(argv[3], NULL, 0);
   
    if ( (send_s=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror ("send socket");
        exit(1);
    }   

    /* If ttl supplied, set it */   

    if (argc == 5) {
        ttl = (u_char)strtol(argv[4], NULL, 0);
    } else {
        ttl = TTL;
    }
    if (setsockopt(send_s, IPPROTO_IP, IP_MULTICAST_TTL,
		&ttl, sizeof(ttl)) < 0) {
        perror ("ttl setsockopt");
        exit(1);
    }

      if (setsockopt(send_s, IPPROTO_IP, IP_MULTICAST_LOOP,
		&yes, sizeof(yes)) < 0) {
        perror ("loop setsockopt");
        exit(1);
    }

      	/*create the LCT header now*/
	lct_infos_m.payload_id_present=1;	
	lct_infos_m.demux_label = 1 ;
	lct_infos_m.idf_toi = 1;
	lct_infos_m.codepoint=0;
	lct_infos_m.cci_idf=0x55001111;//CC_ID_RLC;
	
	
	lct_infos_m.idf_block = 1;	/* BLOCK identifier */
	lct_infos_m.idf_du = 1;		/* DU identifier */
	lct_infos_m.is_fec = 0;		/* 1 if DU has FEC content */
	
	/* fields used by FTI (FEC object tx info) (previously NEWADU) */
	lct_infos_m.adu_len=300;	/* Object length in bytes-unused here- */
	lct_infos_m.block_len=10;	/* full-sized block length in bytes
	                                   -unused here-*/
	
	
	lct_infos_m.NOP_info=1024;
	
	lct_hdr_m   = (fixed_lct_hdr_t*)IOBuf;
	hlen = lct_hdr_generat (choix,(lct_infos_t *)&lct_infos_m,lct_hdr_m);
	if (hlen < 0)
		goto bad;
	if (sendto(send_s, 
               IOBuf, 
	       sizeof(IOBuf), 
	       0,
              (struct sockaddr*)&mcast_group, 
               sizeof(mcast_group)) < sizeof (IOBuf))
	       {
                    perror("sendto");
                    exit(1);}
        printf("\n----------------------------------------------------------------\n");
   	DumpBuffer (IOBuf,sizeof(IOBuf),hlen);
	printf ("The packet sent\n");
	printf("\n----------------------------------------------------------------\n");
  return 0;
  bad:
  return -1;
    
}/*main*/
