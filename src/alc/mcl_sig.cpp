/* $Id: mcl_sig.cpp,v 1.7 2005/01/11 13:12:34 roca Exp $ */
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

/*
 *	signaling functions.
 */

#include "mcl_includes.h"


/* private function */
static int	SendSigPacket	(mcl_cb *mclcb, int type, char *buf, int len);
static void	mcl_sig_initialize (mcl_cb *mclcb);

/*
 * private variables
 */
static	int	initialized = 0;
/* WARNING!: keep it coherent with each signaling type (mcl_alc/lct_hdr.h) */
static short	max_tx_times[SIG_CLOSE+1];


/*
 * high-level SIG sending routines
 */
int
SendNONEWADU (mcl_cb	*mclcb,
	      INT32	max_adu) /* last ADU to be sent */
{
	int	*ptr;
	int	err;

	TRACELVL(5, (mcl_stdout, "-> SendNONEWADU: max_adu=%d\n", max_adu))
	if (mclcb->get_verbosity() >= 1)
		PRINT_OUT((mcl_stdout, "Send_NONEWADU\n"))
	if (!(ptr = (int*)malloc(sizeof(int)))) {
		PRINT_ERR((mcl_stderr, "SendNONEWADU: ERROR, no memory"))
		mcl_exit(-1);
	}
	*ptr = max_adu;
	/* 8 bytes required by EXT_NONEWADU EH, cf mcl_alc_hdr.c */
	err = SendSigPacket(mclcb, EXT_NONEWADU, (char*)ptr, 8);
	if (mclcb->get_stats_level() > 0)
		mcl_print_tx_stats(mclcb);
	TRACELVL(5, (mcl_stdout, "<- SendNONEWADU:\n"))
	return err;
}


int
SendCLOSE (mcl_cb *mclcb)
{
	int	err;

	TRACELVL(5, (mcl_stdout, "-> SendCLOSE:\n"))
#ifdef NEVERDEF
	/* for tests only: check rx FSM if no CLOSE received */
	if (mclcb->get_verbosity() >= 1)
		PRINT_OUT((mcl_stdout, "SendCLOSE deleted!!!\n"))
#else
	if (mclcb->get_verbosity() >= 1)
		PRINT_OUT((mcl_stdout, "Send_CLOSE\n"))
	/* 4 bytes required by SIG_CLOSE EH, cf mcl_alc_hdr.c/mcl_lct_hdr.c */
	err = SendSigPacket(mclcb, SIG_CLOSE, (char*)NULL, 4);
	if (mclcb->get_stats_level() > 0)
		mcl_print_tx_stats(mclcb);
#endif /* NEVERDEF */
	TRACELVL(5, (mcl_stdout, "<- SendCLOSE:\n"))
	return err;
}


/****** SIG manipulation functions ******/


/*
 * 	Add a SIG to the sig_tab.
 *	Return 0 if OK, an error code otherwise.
 */
int
AddSigToTab (mcl_cb	*mclcb,
	     sig_tab_t	*new_sig)
{
	TRACELVL(5, (mcl_stdout, "-> AddSigToTab:\n"))
	/* insert new sig at the begining of list */
	if (mclcb->sig_tab == NULL) {
		new_sig->next = NULL;
		mclcb->sig_tab = new_sig;
	} else {
		/* insert at end; this is preferable for NONEWADU, CLOSE... */
		sig_tab_t	*psig;
		for (psig = mclcb->sig_tab; psig->next != NULL;
		     psig = psig->next); 	
		new_sig->next = NULL;
		psig->next = new_sig;
	}
	mclcb->mcl_sig_pending++;
	TRACELVL(5, (mcl_stdout, "<- AddSigToTab:\n"))
	return 0;
}


/*
 *	Remove a particular sig message from the sig_tab table
 *	Returns 1 if removed, 0 otherwise
 */
int
RemSigFromTab (mcl_cb  *mclcb,
	       int	level,		/* level concerned by this sig hdr */
	       int	type,		/* type of signaling */
	       int	ext_type)
{
#if !defined(ALC)
	sig_tab_t	*psig;

	TRACELVL(5, (mcl_stdout, "-> RemSigFromTab: level=%d, type=%d, ext_type=%d\n",
		level, type, ext_type))
	for (psig = mclcb->sig_tab; psig ; psig = psig->next) {
		if (psig->target_lvl != level || psig->eh_type != type)
			continue;
		switch (type) {
		case EH_SIG: {
			sig_hdr_t	*ph = (sig_hdr_t*)psig->buf;
			if (ph && ph->type == ext_type)
				goto found;
			break;
		}
		}
	}
	TRACELVL(5, (mcl_stdout, "<- RemSigFromTab: none\n"))
	return 0;
found:
	psig->rem2send = 0;
	CleanupSigTab(mclcb);
	TRACELVL(5, (mcl_stdout, "<- RemSigFromTab: removed\n"))
	return 1;
#else
	return 0;
#endif /* !ALC */
}


/*
 *	Required before a calling CopySigToBuf repetedly in mcl_send_pkt
 */
void
CopySigReset(mcl_cb *mclcb)
{
	mclcb->skip = 0;
	mclcb->psig_next = mclcb->sig_tab;
}


/*
 *	Return 1 if there remains some SIG ready to be tx for THIS
 *	layer, 0 otherwise.
 */
int
CanCopyMoreSig (mcl_cb	*mclcb,
		int	level)
{
	sig_tab_t	*psig;

	for (psig = mclcb->psig_next; psig ; psig = psig->next) {
		if (psig->target_lvl == level || psig->target_lvl == -1) {
			/* found at least one */
			return 1;
		}
	}
	return 0;
}


/*
 *	Prepar the copy a given numbers of SIG from the sig_tab but
 *	skip the first ones.
 *	Returns the number of SIG copied if OK, < 0 otherwise.
 */
int
CopySigToLCTinfos (mcl_cb      *mclcb,
		   INT32	level,		/* send pkt here */
		   hdr_infos_t	*hdr_infos,	/* where to copy */
		   INT32	len)		/* available size for new SIG */
{
	int		nb = 0;		/* # of SIG written */
	int		sz = 0;		/* total size written to buf */
	sig_tab_t	*psig;

	TRACELVL(5, (mcl_stdout, "-> CopySigToLCTinfos:\n"))
	ASSERT(level >= 0);
	/*
	 * copy as many SIG as possible
	 * NB: check before that the pending sig's target level is the
	 * right one
	 */
	for (psig = mclcb->psig_next; psig ;
	     mclcb->psig_next = psig = psig->next, mclcb->skip++) {
		if (psig->target_lvl != level && psig->target_lvl != -1) {
			/* not the right target level! skip it... */
			TRACELVL(5, (mcl_stdout, "   CopySigToLCTinfos: bad layer for sig %d (%d, expected %d)\n",
					psig->eh_type, level, psig->target_lvl))
			continue;
		}
		if (psig->rem2send == 0) {
			/* not yet cleaned ! skip it... */
			continue;
		}
		if (sz + psig->len > len) {
			/* can't add any more (no room)! keep it for next time*/
			TRACELVL(5, (mcl_stdout, "   CopySigToLCTinfos: no room for sig %d (%d, expected %d)\n",
					psig->eh_type, len, sz + psig->len))
			//ASSERT(psig->len < MAX_UNFRAG_DATAGRAM_SZ);
			ASSERT(psig->len < MAX_DATAGRAM_SIZE);
			break;
		}
		switch (psig->eh_type) {
		case EXT_NONEWADU:
			hdr_infos->NONEWADU_present = true;
			hdr_infos->max_idf_adu = *(int*)psig->buf;
			break;

		case SIG_CLOSE:
			hdr_infos->close = true;
			break;
		default:
			break;
		}
		TRACELVL(5, (mcl_stdout, "   CopySigToLCTinfos: sig %d added\n",
				psig->eh_type))
		sz += psig->len;
		nb++;
		if (psig->rem2send > 0)
			psig->rem2send--;
	}
	TRACELVL(5, (mcl_stdout, "<- CopySigToLCTinfos: nb=%d\n", nb))
	return nb;
}


/*
 *	Withdraw all the SIG sent a sufficient number of times from the sig_tab.
 *	Return 0 if OK, an error code otherwise.
 */
int
CleanupSigTab (mcl_cb	*mclcb)
{
	sig_tab_t	*psig,
			*prev;

	TRACELVL(5, (mcl_stdout, "-> CleanupSigTab:\n"))
	for (psig = mclcb->sig_tab, prev = NULL; psig != NULL; ) {
		/*
		 * if psig->rem2send < 0 then tx for ever
		 * else if == 0, remove it,
		 * else if > 0, keep it.
		 */
		if (psig->rem2send == 0) {
			free(psig->buf);
			if (psig->saddr && psig->saddr_len > 0)
				free(psig->saddr);
			if (psig == mclcb->sig_tab) {
				mclcb->sig_tab = psig->next;
				free(psig); 
				psig = mclcb->sig_tab;
			} else {
				ASSERT(prev)
				prev->next = psig->next;
				free(psig);
				psig = prev->next;	
			}
			mclcb->mcl_sig_pending--;
		} else {
			prev = psig;
			psig = psig->next;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- CleanupSigTab:\n"))
	return 0;
}


/*
 * Send a signaling message.
 * Depending on the packet type, this may happen either in a future
 * data packet or immediately (e.g. with EXT_NONEWADU, SIG_CLOSE).
 */
static int
SendSigPacket (mcl_cb	*mclcb,
	       int	type,
	       char	*buf,
	       int	len)
{
	sig_tab_t	*new_sig;
	int		tmp_rem2send;

	TRACELVL(5, (mcl_stdout, "-> SendSigPacket: type=%d, buf=x%x, len=%d\n",
		type, (int)buf, len))
	ASSERT(type != SIG_CLOSE || buf == NULL);	/* no buf with CLOSE */
	if (!initialized)
		mcl_sig_initialize(mclcb);
	if (!(new_sig = (sig_tab_t *)malloc(sizeof(sig_tab_t)))) {
		PRINT_ERR((mcl_stderr, "SendSigPacket: ERROR, no memory"))
		mcl_exit(-1);
	}
	new_sig->next	= (sig_tab_t*) NULL;
	new_sig->eh_type= type;
	new_sig->buf	= buf;
	new_sig->len	= len;
	new_sig->target_lvl	= 0;			/* send on layer 0 */
	new_sig->saddr	= NULL;			/* by default */
	new_sig->saddr_len	= 0;			/* by default */
	new_sig->rem2send	= max_tx_times[type];
	AddSigToTab(mclcb, new_sig);
	if (type == SIG_CLOSE && mclcb->nb_layers > 0) {
		tmp_rem2send = new_sig->rem2send;
		do {	/* tx immediately at least once */
			/* copy var to tmp as mcl_send_pkt could free block */
			mcl_send_pkt(mclcb, 0, NULL, NULL);
			tmp_rem2send--;
		} while (tmp_rem2send > 0);
		CleanupSigTab(mclcb);
	}
	TRACELVL(5, (mcl_stdout, "<- SendSigPacket:\n"))
	return 0;
}


/*
 * remove and free everything
 */
void
mcl_sig_close (mcl_cb	*mclcb)
{
	sig_tab_t	*psig,
			*prev;

	TRACELVL(5, (mcl_stdout, "-> mcl_sig_close:\n"))
	for (psig = mclcb->sig_tab, prev = NULL; psig != NULL; ) {
		if (psig->buf) {
			free(psig->buf);
			psig->buf = NULL;
		}
		if (psig->saddr && psig->saddr_len > 0) {
			free(psig->saddr);
			psig->saddr = NULL;
		}
		if (psig == mclcb->sig_tab) {
			mclcb->sig_tab = psig->next;
			free(psig); 
			psig = mclcb->sig_tab;
		} else {
			ASSERT(prev)
			prev->next = psig->next;
			free(psig);
			psig = prev->next;	
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_sig_close:\n"))
}


/****** Private Functions *****************************************************/


/*
 * initialize everything
 *
 * WARNING!: keep it coherent with each signaling type (mcl_alc/lct_hdr.h)
 */
static void
mcl_sig_initialize (mcl_cb	*mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_sig_initialize:\n"))
	ASSERT(!initialized);
	memset(max_tx_times, 0, sizeof(max_tx_times));
	max_tx_times[EXT_NONEWADU] = (short)MAX_TX_EXT_NONEWADU;
	max_tx_times[SIG_CLOSE] = (short)MAX_TX_SIG_CLOSE;
	initialized = 1;
	TRACELVL(5, (mcl_stdout, "<- mcl_sig_initialize:\n"))
}

