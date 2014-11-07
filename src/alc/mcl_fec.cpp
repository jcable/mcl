/* $Id: mcl_fec.cpp,v 1.35 2005/05/17 12:36:57 roca Exp $ */
/*
 *  Copyright (c) 2004 INRIA - All rights reserved
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


#ifdef LDPC_FEC
/* a define specific to LDGM/LDPC */
/*#define LEFT_DEGREE	10*/
#define LEFT_DEGREE	3
#endif


mcl_fec::mcl_fec ()
{
	/* set everything to 0 */
	memset(this->max_k, 0, sizeof(this->max_k));
	memset(this->max_n, 0, sizeof(this->max_n));
	memset(this->cur_k, 0, sizeof(this->cur_k));
	memset(this->cur_n, 0, sizeof(this->cur_n));
	/*
	 * initialize everything
	 */
	this->initialize();
}


mcl_fec::~mcl_fec ()
{
#ifdef DEBUG
	memset(this->max_k, 0, sizeof(this->max_k));
	memset(this->max_n, 0, sizeof(this->max_n));
	memset(this->cur_k, 0, sizeof(this->cur_k));
	memset(this->cur_n, 0, sizeof(this->cur_n));
#endif
}


/**
 * Initializes the FEC class for a given FEC code.
 * => See header file for more informations.
 */
void
mcl_fec::initialize ()
{
	/*
	 * Initialize the fec class for each supported FEC code.
	 * The order is significant since the last call specifies the
	 * default FEC code.
	 */
	/* MCL_FEC_SCHEME_NULL case */
	// no fec codec used, so use default conservative values...
	this->fec_codec = MCL_FEC_SCHEME_NULL;
	this->max_k[MCL_FEC_SCHEME_NULL] = 255;
	this->max_n[MCL_FEC_SCHEME_NULL] = 255;
	this->cur_k[MCL_FEC_SCHEME_NULL] = 255;
	this->cur_n[MCL_FEC_SCHEME_NULL] = 255;
#ifdef LDPC_FEC
	/* MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1 case */
	this->fec_codec = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
	this->max_k[MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1] = LDPC_MAX_K;
	this->max_n[MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1] = LDPC_MAX_N;
	this->cur_k[MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1] = LDPC_DEFAULT_K;
	this->cur_n[MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1] = LDPC_DEFAULT_N;
	/* MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0 case */
	this->fec_codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
	this->max_k[MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0] = LDPC_MAX_K;
	this->max_n[MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0] = LDPC_MAX_N;
	this->cur_k[MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0] = LDPC_DEFAULT_K;
	this->cur_n[MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0] = LDPC_DEFAULT_N;
#endif
#ifdef RSE_FEC
	/* MCL_FEC_SCHEME_RSE case */
	this->fec_codec = MCL_FEC_SCHEME_RSE_129_0;
	ASSERT(RSE_MAX_K <= RSE_MAX_N);
	ASSERT(RSE_MAX_N <= GF_SIZE);
	this->max_k[MCL_FEC_SCHEME_RSE_129_0] = RSE_MAX_K;
	this->max_n[MCL_FEC_SCHEME_RSE_129_0] = RSE_MAX_N;
	this->cur_k[MCL_FEC_SCHEME_RSE_129_0] = RSE_DEFAULT_K;
	this->cur_n[MCL_FEC_SCHEME_RSE_129_0] = RSE_DEFAULT_N;
#endif
}


/**
 * Set the FEC code to be used.
 * This function is sender specific (a receiver gets the FEC
 * codec information info from the various incoming packet fields).
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::set_fec_code (mcl_cb	*const mclcb,
		      const INT32	new_code)
{
	switch (new_code) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		this->fec_codec = MCL_FEC_SCHEME_RSE_129_0;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use RSE\n"))
		return MCL_OK;
#endif

#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
		this->fec_codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use LDPC-STAIRCASE\n"))
		return MCL_OK;

	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		this->fec_codec = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use LDPC-TRIANGLE\n"))
		return MCL_OK;
#endif

	case MCL_FEC_SCHEME_NULL:
		this->fec_codec = MCL_FEC_SCHEME_NULL;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use NO FEC\n"))
		return MCL_OK;


	default:
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_fec_code: ERROR, code %d unknown\n", new_code))
		return MCL_ERROR;
	}
}


/**
 * Return the FEC code string corresponding to the current FEC codec
 * used.
 * WARNING: non re-entrant, the string is statically allocated...
 * @return		string with the name of the FEC code.
 */
char*
mcl_fec::get_fec_code_string () const
{
	switch (this->get_fec_code ()) {
	case MCL_FEC_SCHEME_RSE_129_0:
		return((char*)"Reed-Solomon FEC codec");
	case MCL_FEC_SCHEME_NULL:
		return((char*)"NULL FEC codec");
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
		return((char*)"LDGM_STAIRCASE FEC codec");
	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		return((char*)"LDGM_TRIANGLE FEC codec");
	default:
		return((char*)"ERROR, unknown FEC codec");
	}
}


/**
 * Set the current FEC ratio, defined as cur_n/cur_k, and update
 * either cur_n or cur_k, depending on the FEC codec.
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::set_fec_ratio (mcl_cb	*const mclcb,
			double	new_fec_ratio)
{

	if (new_fec_ratio < 1.0) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_fec_ratio: ERROR, bad new_fec_ratio (%f), must be >= 1.0\n", new_fec_ratio))
		return MCL_ERROR;
	}
	switch (this->fec_codec) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		return (this->set_fec_ratio_from_n(mclcb, new_fec_ratio));
#endif

#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
		return (this->set_fec_ratio_from_k(mclcb, new_fec_ratio));

	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		return (this->set_fec_ratio_from_k(mclcb, new_fec_ratio));
#endif

	case MCL_FEC_SCHEME_NULL:
		return (this->set_fec_ratio_from_n(mclcb, new_fec_ratio));

	default:
		mcl_exit(-1);	// impossible !
		return MCL_ERROR;	// to avoid warning
	}
}


mcl_error_status
mcl_fec::set_fec_ratio_from_n (mcl_cb	*const mclcb,
			       double	new_fec_ratio)
{ 

	INT32	tmp_cur_k;	// used with CODE_NULL and RSE

	tmp_cur_k = (INT32)((double)this->get_n() / new_fec_ratio);
	if (tmp_cur_k < 1) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_fec_ratio_from_n: ERROR, new_fec_ratio %f too large\n", new_fec_ratio))
		return MCL_ERROR;
	}
	this->set_k(tmp_cur_k);
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::set_fec_ratio_from_n: max_k/n=(%d; %d), cur_k/n=(%d; %d), ratio=%f\n",
		this->get_max_k(), this->get_max_n(),
		this->get_k(), this->get_n(), this->get_fec_ratio()))
	return MCL_OK;
}


mcl_error_status
mcl_fec::set_fec_ratio_from_k (mcl_cb	*const mclcb,
			       double	new_fec_ratio)
{
	INT32	tmp_cur_n;	// used with LDGM and LDPC

	tmp_cur_n = (INT32)((double)this->get_k() * new_fec_ratio);
	if (tmp_cur_n > this->get_max_n()) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_fec_ratio: ERROR, new_fec_ratio %f too large, would causes the cur_n (%d) to be larger than max_n (%d)\n",
		new_fec_ratio, tmp_cur_n, this->get_max_n()))
		return MCL_ERROR;
	}
	this->set_n(tmp_cur_n);
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::set_fec_ratio_from_n: max_k/n=(%d; %d), cur_k/n=(%d; %d), ratio=%f\n",
		this->get_max_k(), this->get_max_n(),
		this->get_k(), this->get_n(), this->get_fec_ratio()))
	return MCL_OK;
}


/**
 * Converts from FEC Encoding ID/FEC Instance ID to FEC scheme.
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::enc_inst_to_scheme (class mcl_cb	*const mclcb,
			     UINT8		encid,
			     UINT8		instid,
			     mcl_fec_scheme	*scheme)
{
	switch (encid) {
	case FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC:
#ifdef RSE_FEC
		if (instid != 0) {
			goto bad;
		}
		*scheme = MCL_FEC_SCHEME_RSE_129_0;
		break;
#endif

#ifdef LDPC_FEC
	case FEC_ENCODING_ID_PCM_FEC:
		if (instid == 0) {
			*scheme = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
		} else if (instid == 1) {
			*scheme = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
		} else {
			goto bad;
		}
		break;
#endif

	case FEC_ENCODING_ID_NO_FEC:
		if (instid != 0) {
			goto bad;
		}
		*scheme = MCL_FEC_SCHEME_NULL;
		break;


	default:
		goto bad;
	}
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::enc_inst_to_scheme: FEC Enc ID/Inst ID=(%d/%d), scheme is %d\n",
		encid, instid, (INT32)*scheme))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr,
	"mcl_fec::enc_inst_to_scheme: ERROR, unknown FEC Encoding ID/FEC Instance ID (%d, %d)\n",
		encid, instid))
	return MCL_ERROR;
}


/**
 * Converts from FEC scheme to FEC Encoding ID/FEC Instance ID.
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::scheme_to_enc_inst (class mcl_cb	*const mclcb,
			     mcl_fec_scheme	scheme,
			     UINT8		*encid,
			     UINT8		*instid)
{
	switch (scheme) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		*encid = FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC;
		*instid = 0;
		break;
#endif

#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
		*encid = FEC_ENCODING_ID_PCM_FEC;
		*instid = 0;
		break;

	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		*encid = FEC_ENCODING_ID_PCM_FEC;
		*instid = 1;
		break;
#endif

	case MCL_FEC_SCHEME_NULL:
		*encid = FEC_ENCODING_ID_NO_FEC;
		*instid = 0;
		break;

	default:
		PRINT_ERR((mcl_stderr,
		"mcl_fec:: enc_inst_to_scheme: ERROR, unknown scheme (%d)\n",
			scheme))
		return MCL_ERROR;
	}
	return MCL_OK;
}


/**
 * Return the number of FEC packets that can still be created for this
 * block.
 * => See header file for more informations.
 */
INT32
mcl_fec::get_rem_nb_fec_pkts_to_create (mcl_cb		*const mclcb,
					block_t		*blk)
{
#ifdef FEC
	return (this->get_n() - blk->k - blk->fec_du_nb_in_list);
#else
	return 0;
#endif
}


/**
 * Determine the number of FEC packets required for this block.
 * Depends on the block size (k) and the kind of scheduling used
 * for this session.
 * => See header file for more informations.
 */
INT32
mcl_fec::compute_n_for_this_block (mcl_cb	*const mclcb,
				   INT32		k)
{
	ASSERT(k > 0);
	ASSERT(k <= this->get_max_k());
	switch(mclcb->scheduler) {
	case MCL_SCHED_LCT1:
		{
		int	n;
		//n = min ((int)floor(this->get_fec_ratio() * k), this->get_n());
		if (k == this->get_k()) {
			/*
			 * full-sized block;
			 * ignore the FEC ratio here since it is already
			 * considered in get_n())
			 */
			n = this->get_n();
		} else {
			/*
			 * non full-sized block;
			 * consider the FEC ratio here
			 */
			n = (int)floor((double)this->get_fec_ratio() * (double)k);
		}
		TRACELVL(4, (mcl_stdout,
			"   compute_n_for_this_block: SCHED1, k=%d n=%d, %d FEC DUs, cur_k=%d, cur_n=%d\n",
			k, n, n - k, this->get_k(), this->get_n()))
		return(n);
		}
	default:
		PRINT_ERR((mcl_stderr,
		"mcl_fec::compute_n_for_this_block: ERROR, unknown scheduler\n"))
		mcl_exit(-1);
		return(1);	// avoid warnings!
	}
}

#ifdef FEC
/**
 * Encode the block and create the appropriate number of new FEC packets.
 * => See header file for more informations.
 */
INT32
mcl_fec::encode (mcl_cb	*const mclcb,
		 block_t	*blk)
{
	INT32		k;		// effective # source DUs for this block
					// FEC DU seq nbs follow source DUs
	INT32		n;		// total of n (FEC + source) DUs
	INT32		fec_seq;	// FEC packet sequence number
	UINT32 		du_len;
	UINT32		last_du_len;
	INT32		i;
#ifdef RSE_FEC
	void		*code = NULL;	/* ptr to the RSE FEC object */
#endif
#ifdef LDPC_FEC
	LDPCFecSession	*ldpc_ses = NULL; /* ptr to the FEC session class */
#endif
	du_t		*fec_du;
	char		*fec_data;
	void		**pkt_canvas = NULL; /* table of ptrs to packets */
	 			/* With RSE, a table of k entries is enough, */
	 			/* with LDGM, a table of k entries is enough, */
	 			/* BUT with LDPC the table must have n entries*/
	du_t		*du;

	if (this->fec_codec == MCL_FEC_SCHEME_NULL) {
		TRACELVL(5, (mcl_stdout,
		"   mcl_fec::encode(NULL): useless, no FEC mode, return 0\n"))
		return 0;
	}
	ASSERT(blk);
	/*
	 * Sanity checks...
	 */
	k = blk->k;
	ASSERT(k <= this->get_k());
	n = this->compute_n_for_this_block(mclcb, k);
	ASSERT(n <= this->get_n());
	/*
	 * see if some FEC is still required
	 */
	if (n == k) {
		TRACELVL(5, (mcl_stdout,
		"   mcl_fec::encode(NULL): useless, no FEC required, return 0\n"))
		return 0;
	}
	/*
	 * compute the size of a full-sized DU and of the last DU
	 * which may be shorter
	 */ 
	//du_len = mclcb->get_payload_size();
	//du_len = mclcb->payload_size;
	du_len = blk->adu->symbol_len;
	last_du_len = (k > 1) ? blk->len % du_len : blk->len;
	if (last_du_len == 0)
		last_du_len = du_len;	/* multiple */
	/*
	 * create all the required FEC DUs for this block
	 */
	switch (this->fec_codec) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		TRACELVL(5, (mcl_stdout, "-> mcl_fec::encode(RSE): blk seq=%d, k=%d, n=%d\n", blk->seq, k, n))
		if (!(code = fec_new(k,n))) {
			goto no_memory;
		}
		break;
#endif
#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		TRACELVL(5, (mcl_stdout, "-> mcl_fec::encode(LDPC): blk seq=%d, k=%d, n=%d, key=%d\n", blk->seq, k, n, blk->fec_key))
		ldpc_ses = new LDPCFecSession;
		/*
		 * NB: fec_key initialization moved to ADU segmentation
		 * function in order to do it immediately, at ADU submission.
		 * It's important if there's a risk the FTI be transmitted
		 * before mclcb->fec.encode() is called!
		 * Besides the key must be the same for all blocks because
		 * of FTI limitations (only one value carried).
		 */
		if (ldpc_ses->InitSession(k, n-k, du_len, FLAG_CODER,
					  min(LEFT_DEGREE, n-k),
					  blk->fec_key,
					  TypeSTAIRS) == LDPC_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::encode(LDPC): ERROR, LDPCFecSession::InitSession failed\n"))
			mcl_exit(-1);
		}
#ifdef DEBUG
		if (mclcb->get_verbosity() >= 2) {
			ldpc_ses->SetVerbosity(1);
		}
#endif
		break;
#endif
	default:
		mcl_exit(-1);	// NULL FEC not possible here
	}

#ifdef VIRTUAL_TX_MEM
	if (mclcb->vtm_used) {
		/*
		 * first, move data in memory; assumes VTMEM cache is large
		 * enough to contain the whole block data!
		 */
		/* XXX: not efficient, must get it from disk... */
		char *data;
		for (i = 0; i < k; i++) {
			data = mcl_vtm_get_data(mclcb, &(blk->du_head[i]));
			ASSERT(data);
			/* check that VTMEM cache is sufficiently large */
			if (blk->du_head[i].data == NULL) {
				PRINT_ERR((mcl_stderr, "mcl_fec::encode: ERROR, Virtual Tx Memory cache too small\n"))
				mcl_exit(-1);
			}
		}
	}
#endif  /* VIRTUAL_TX_MEM */

	/*
	 * no data copy except for the last du which may be shorter.
	 * for the last DU, (c)allocate a block, of "du_len" size, and copy
	 * the "last_du_len" bytes in it.
	 */
	switch (this->fec_codec) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		if (!(pkt_canvas = (void**)malloc(k * sizeof(char*)))) {
			goto no_memory;
		}
		break;
#endif
#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		if (!(pkt_canvas = (void**)calloc(n, sizeof(char*)))) {
			goto no_memory;
		}
		break;
#endif
	default:
		mcl_exit(-1);	// NULL FEC not possible here
	}
	for (i = 0, du = blk->du_head; i < k - 1; i++, du++) {
		pkt_canvas[i] = (void*)(du->data);
	}
	if (!(pkt_canvas[k-1] = (void*)calloc(1, du_len))) {
		goto no_memory;
	}
	ASSERT(du->seq == (UINT32)k-1);
	ASSERT(du->len == last_du_len);
	memcpy(pkt_canvas[k-1], du->data, last_du_len);
	/*
	 * do it simply: allocate a tab of FEC du_t structs
	 * rather than a linked list!
	 * XXX: cannot be used if additional FEC packets are created later...
	 */
	ASSERT(blk->fec_du_head == NULL);
	if (!(blk->fec_du_head = (du_t*)calloc(n-k, sizeof(du_t)))) {
		goto no_memory;
	}
	blk->fec_du_nb_in_list = n-k;		/* n-k fec added */
	
	/*
	 * now create the n-k FEC DUs
	 */
	for (fec_seq = k,  fec_du = blk->fec_du_head;
	     fec_seq < n;
	     fec_seq++, fec_du++) {
		if (!(fec_data = (char *)malloc(du_len))) {
			goto no_memory;
		}
		switch (this->fec_codec) {
#ifdef RSE_FEC
		case MCL_FEC_SCHEME_RSE_129_0:
			fec_encode(code, pkt_canvas, fec_data, fec_seq, du_len);
			break;
#endif
#ifdef LDPC_FEC
		case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
		case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
			//TRACE((mcl_stdout,
			//	"mcl_fec::encode: fec_seq=%d, fec_data=x%x, du_len=%d\n",
			//	fec_seq, (int)fec_data, du_len))
			ldpc_ses->BuildFecPacket(pkt_canvas, fec_seq - k, (void*)fec_data);
			// store packet in the canvas
			pkt_canvas[fec_seq] = (void*)(fec_data);
			break;
#endif
		default:
			mcl_exit(-1);	// NULL FEC not possible here
		}
		//fec_du = new mcl_du;
		// fec_du->next = fec_du->prev = NULL;
		fec_du->block	= blk;
		fec_du->data 	= fec_data;
		fec_du->seq	= fec_seq;
		fec_du->len 	= du_len;
		fec_du->is_fec	= true;
		//fec_du->pkt	= NULL;	// this is how a tx DU is distinguished
					// from a rx DU!
		//fec_du->set_next(NULL);
		//fec_du->set_prev(NULL);
#if 0
		/*
		 * Use a linked list of FEC DUs rather than a single tab as
		 * with data DUs on the sending side!
		 */
		if (blk->insert_in_fec_du_list(mclcb, fec_du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr, "mcl_fec::encode: ERROR, insert_in_fec_du_list() failed\n"))
			mcl_exit(-1);
		}
#endif
#ifdef VIRTUAL_TX_MEM
		fec_du->vtm_info.du_in_seq_in_txtab = 1; /* by default */
		if (mcl_vtm_can_store_in_vtm(mclcb, (int)du_len)) {
			/*
			 * use the VTMEM service to register data
			 */
			if (mcl_vtm_store_data(mclcb, NULL, fec_du, fec_data,
						(INT32)du_len, 0)) {
				PRINT_ERR((mcl_stderr,
				"mcl_sendto: ERROR: Virtual Tx Memory service failed\n"))
				mcl_exit(-1);
			}
			free(fec_data);		/* no longer needed */
		} else {
#endif /* VIRTUAL_TX_MEM */
			/*
			 * store in physical memory
			 */
			fec_du->data 	= fec_data;
#ifdef VIRTUAL_TX_MEM
			if (mclcb->vtm_used) {
				/* remember it is kept in physical memory */
				mcl_vtm_register_in_ptm(mclcb, NULL, fec_du,
							(int)du_len);
			}
		}
#endif  /* VIRTUAL_TX_MEM */

		TRACELVL(5, (mcl_stdout,
			"   mcl_fec::encode: created FEC seq=%d, len=%d, buf=x%x\n",
			(INT32)fec_du->seq, (INT32)fec_du->len,
			(INT32)fec_du->data))
	}
	free(pkt_canvas[k-1]);
	free(pkt_canvas);
	switch (this->fec_codec) {
#ifdef RSE_FEC
	case MCL_FEC_SCHEME_RSE_129_0:
		fec_free(code);
		break;
#endif
#ifdef LDPC_FEC
	case MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0:
	case MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1:
		ldpc_ses->EndSession();
		delete ldpc_ses;
		break;
#endif
	default:
		mcl_exit(-1);	// NULL FEC not possible here
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::encode: %d fec_du\n", n - k))
	return (n - k);

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_fec::encode: ERROR, no memory\n"))
	mcl_exit(-1);
	return -1;	// useless but avoids a warning
}
#endif /* FEC */



#ifdef RSE_FEC
/**
 * Decode the block immediately.
 * This function is for Reed-Solomon and similar FEC codes.
 * => See header file for more informations.
 */
INT32
mcl_fec::decode (mcl_cb		*const mclcb,
		 block_t	*blk)
{
	INT32 	k;
	INT32 	n;
	UINT32	du_len;		/* full-sized DU length */
	UINT32	last_du_len;	/* last DU (true) length */
	INT32	i;
	u_char	**dst;		/* put rx data or fec here... */
	INT32	rx_index[GF_SIZE]; /* ... and their seq# here */
	INT32	idx;		/* index in dst[] and rx_index[] tabs */
	UINT32 	rem;		/* remaining nb of DUs */
	void 	*code;
	du_t	*du;
	du_t	*ndu;
	UINT32	seq;		/* next seq number expected */
	mcl_rx_pkt	*pkt;	/* used for data DUs created during decoding */
	u_char	*buf;		/* buffer where data/FEC DU data is copied */
	INT32	off;		/* offset */

	ASSERT(blk);
	TRACELVL(5, (mcl_stdout, "-> mcl_fec::decode(RSE): block seq=%d\n",blk->seq))
	/* make sure RSE is registered... required by some FEC functions 
	 * like compute_n_for_this_block() */
	//this->set_fec_code (mclcb, MCL_FEC_SCHEME_RSE_129_0);
	this->fec_codec = MCL_FEC_SCHEME_RSE_129_0;
	/*
	 * check that FEC decode is indeed needed (i.e. that
	 * we need at least one FEC DU)
	 */
	k = blk->k;
	n = blk->n;
	ASSERT(n >= k);
	//if (blk->get_du_nb_in_list() >= k)
	if ((int)blk->du_nb_in_list >= k) {
		//blk->set_rx_status(BLK_STATUS_DECODED);
		blk->rx_status = BLK_STATUS_DECODED;
		TRACELVL(5, (mcl_stdout,
			"   mcl_fec::decode(RSE): decoding not needed\n"))
		/*TRACELVL(5, ("mcl_fec::decode(RSE): k=%d, du_in_list=%d, fec_in_list=%d\n", k, blk->get_du_nb_in_list(), blk->get_fec_du_nb_in_list()))*/
		goto free_fec_du;
	}
	/*
	 * compute the size of a full-sized DU and of the last DU
	 * which may be shorter
	 */ 
	//du_len = mclcb->get_payload_size();
	du_len = blk->adu->symbol_len;
	last_du_len = (k > 1) ? blk->len % du_len : blk->len;
	if (last_du_len == 0)
		last_du_len = du_len;	/* multiple */
	/*
	 * copy data and fec in dst buffer array first
	 * NB: this is required by the current FEC codec which modifies
	 * the dst buffers!!!
	 */
	if (!(dst = (u_char **) malloc(k * sizeof(u_char *)))) {
		goto no_memory;
	}
	/* alloc a single large buf, where to copy all data/FEC DU data... */
	if (!(buf = (u_char *) malloc(k * du_len))) {
		goto no_memory;
	}
	/* and remember the location of each DU buffer */
	for (i = 0, off = 0; i < k; i++, off += du_len) {
		dst[i] = buf + off;
	}
	idx = 0;			/* index in rx_index[] */
	/*
	 * copy data DU to the dst array first
	 */
	//for (rem = blk->get_du_nb_in_list(), du = blk->get_du_head();
	//     rem > 0; idx++, rem--, du = du->get_next())
	for (rem = blk->du_nb_in_list, du = blk->du_head;
	     rem > 0; idx++, rem--, du = du->next) {
		ASSERT(du);
		/* WARNING: if the following test failed, you probably forgot */
		/* to set the same tx_profile at the source AND receiver */
		if (du->len != du_len && du->len != last_du_len) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: bad packet length (expected %d or %d, got %d)\nCheck transmission profiles at sender/receiver\n", 
				du_len, last_du_len, du->len))
			goto fatal_error;
		}
		if (idx >= k)
			break;	/* security, eg. if there are more than k DUs */
		if (mclcb->rx_storage.get_data(mclcb, du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: rx_storage.get_data() failed\n")) 
			goto fatal_error;
		}
		memcpy(dst[idx], du->data, du->len);
		if (du->len < du_len) {
			/* non full-sized DU, so reset the remaining bytes */
			memset(dst[idx] + du->len, 0, du_len - du->len);
		}
		rx_index[idx] = du->seq;
#if 0
		PRINT_OUT((mcl_stdout, "mcl_fec::decode(RSE): copy DATA du->seq=%d in dst[%d], rx_index[%d]=%d\n",
			du->seq, idx, idx, rx_index[idx]))
#endif
	}
	/*
	 * copy FEC DUs to the dst array now
	 */
	//for (rem = blk->get_fec_du_nb_in_list(), du = blk->get_fec_du_head();
	//     rem > 0; idx++, rem--, du = du->get_next())
	for (rem = blk->fec_du_nb_in_list, du = blk->fec_du_head;
	     rem > 0; idx++, rem--, du = du->next) {
		ASSERT(du);
		if (du->len != du_len) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: bad FEC packet length (expected %d or %d, got %d)\nCheck transmission profiles at sender/receiver\n", 
				du_len, last_du_len, du->len))
			goto fatal_error;
		}
		ASSERT(du->len == du_len);
		if (idx >= k) {
			/* security, if there are more than k data+FEC DUs */
			break;
		}
		if (mclcb->rx_storage.get_data(mclcb, du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: rx_storage.get_data() failed\n")) 
			goto fatal_error;
		}
		memcpy(dst[idx], du->data, du->len);
		/* rx_index[idx] = du->seq + k; */
		rx_index[idx] = du->seq;
#if 0
		PRINT_OUT((mcl_stdout, "mcl_fec::decode(RSE): copy FEC du->seq=%d in dst[%d], rx_index[%d]=%d\n",
			du->seq, idx, idx, rx_index[idx]))
#endif
	}
	/*
	 * now decode
	 * NB: all the dst packets must be du_len long
	 */
	code = fec_new(k, n);
	if (fec_decode(code, (void**)dst, rx_index, du_len)) {
		PRINT_ERR((mcl_stderr, "mcl_fec::decode(RSE): ERROR, fec_decode failed\n"))
		TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode(RSE): ERROR, calculated k/n=(%d, %d), cur_k/cur_n=(%d, %d)\n", k, n, this->get_k(), this->get_n()))
		fec_free(code);
		return -1;
	}
	fec_free(code);

	/*
	 * now update the block with the rx or reconstructed DUs
	 */
	//for (rem = blk->k, seq = 0, du = blk->get_du_head();
	//     rem > 0; rem--, seq++, du = du->get_next()) 
	for (rem = blk->k, seq = 0, du = blk->du_head;
	     rem > 0; rem--, seq++, du = du->next) {
		if (du && du->seq == seq) {
			/* nothing to do, DU already received */
			continue;
		}
		/*
		 * else copy it from the FEC decoded matrix
		 */
		//du = new mcl_du;
		du = CreateDU(mclcb);
		/* next = prev = NULL */
		du->block	= blk;
		du->seq		= seq;
		if (rem > 1)
			du->len	= du_len;
		else
			du->len	= last_du_len;
		du->is_fec 	= false;
		//du->set_next(NULL);
		//du->set_prev(NULL);
		/*
		 * copy/store the decoded data now
		 */
		pkt = new mcl_rx_pkt (du->len);
		ASSERT(pkt);
		pkt->pkt_len = du->len;
		du->pkt = pkt;
		du->data = pkt->get_buf();
		memcpy(du->data, dst[seq], du->len);
		if (mclcb->rx_storage.store_data(mclcb, du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: rx_storage.store_data() failed\n")) 
			goto fatal_error;
		}
		/*
		 * and insert it in the data DU list
		 */
		//if (blk->insert_in_du_list(mclcb, du) == MCL_ERROR) {
		//	PRINT_ERR((mcl_stderr, "mcl_fec::decode(RSE): ERROR, insert_in_fec_du_list() failed\n"))
		//	mcl_exit(-1);
		//}
		InsertDU(mclcb, du, &(blk->du_head));
		blk->du_nb_in_list++;
		blk->adu->recvd_src_data += du->len;
		TRACELVL(5, (mcl_stdout,
			"   mcl_fec::decode(RSE): created DU seq=%d from dst[%d], len=%d, du->data=x%x\n",
			du->seq, seq, du->len, (int)du->data))
	}
	//blk->set_rx_status(BLK_STATUS_DECODED);
	blk->rx_status = BLK_STATUS_DECODED;
	/*
	 * free the dst[] array and fec DU list now
	 */
	/*
	 * Warning: the fec_decode function can shuffle dst[] entries, so
	 * the new dst[0] entry may not point to buffer allocated previously!
	 */
	free(buf);
	free(dst);
free_fec_du:
	//blk->remove_and_free_all_fec_dus(mclcb);
	for (rem = blk->fec_du_nb_in_list, du = blk->fec_du_head; rem > 0;
	     rem--, du = ndu) {
		ndu = du->next;
		if (mclcb->rx_storage.free_data(mclcb, du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(RSE): ERROR: rx_storage.free_data() failed\n")) 
			goto fatal_error;
		}
		free(du);
	}
	blk->fec_du_head = NULL;
	blk->fec_du_nb_in_list = 0;

	TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode(RSE): ok\n"))
	return 0;

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_fec::decode(RSE): ERROR, no memory"))
	mcl_exit(-1);

fatal_error:
	PRINT_ERR((mcl_stderr, "mcl_fec::decode(RSE): ERROR"))
	mcl_exit(-1);
	return -1;	/* unreachable; avoid a compiler warning */
}
#endif /* RSE_FEC */


#ifdef LDPC_FEC
/**
 * Progress in the block decoding with the given packet.
 * This function is for LDPC and similar FEC codes who use an
 * iterative decoding approach.
 * => See header file for more informations.
 */
INT32
mcl_fec::decode (mcl_cb		*const mclcb,
		 du_t		*rx_du)
{
	block_t		*blk;
	UINT32 		k;
	UINT32 		n;
	UINT32		du_len;		// full-sized DU length
	//UINT32	last_du_len;	// last DU (true) length
	//INT32		i;
	INT32		rem;		// remaining nb of DUs in iter loop
	du_t		*du;		// current DU in DU iteration loop
	du_t		*ndu;		// next DU in the DU iteration loop

	ASSERT(rx_du);
	blk = rx_du->block;
	ASSERT(blk);
	TRACELVL(5, (mcl_stdout,
		"-> mcl_fec::decode(LDPC-*): du->seq=%d, block seq=%d\n",
		rx_du->seq, blk->seq))
	/* make sure LDGM* is registered... required by some FEC functions 
	 * like compute_n_for_this_block() */
	//this->set_fec_code (mclcb, MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0);
	//this->fec_codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
	ASSERT((this->fec_codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0) ||
	       (this->fec_codec == MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1));
	/*
	 * check that FEC decode is indeed needed (i.e. that
	 * we need at least one FEC DU)
	 */
	k = blk->k;
	n = blk->n;
	ASSERT(n >= k);
	if (k == n && blk->du_nb_in_list >= k) {
		/*
		 * decoding is over for this block which in fact does not
		 * use FEC encoding (no parity symbol, k == n).
		 */
		//blk->set_rx_status(BLK_STATUS_DECODED);
		blk->rx_status = BLK_STATUS_DECODED;
		ASSERT(blk->ldpc_ses == NULL);
		ASSERT(blk->pkt_canvas == NULL);
		ASSERT(blk->callback_ctxt == NULL);
		ASSERT(blk->fec_du_nb_in_list == 0);
		ASSERT(blk->fec_du_head == NULL);
		TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode(LDPC-*): ok but FEC not used\n"))
		return 1;
	}
	/*
	 * LDPC initialization if applicable
	 */
	if (blk->ldpc_ses == NULL) {
		/*
		 * first call to this LDPC decoding function for this
		 * block, so create the LDCP context and packet table.
		 */
		du_len=blk->adu->symbol_len;
		ASSERT(blk->pkt_canvas == NULL);
		blk->ldpc_ses = new LDPCFecSession;
		/*
		 * XXX: the initial seed to rand() must be random! Change it
		 * and give the info to the receiver...
		 */
		if (blk->ldpc_ses->InitSession(k, n-k, du_len, FLAG_DECODER,
						min(LEFT_DEGREE, n-k),
						blk->fec_key,
						TypeSTAIRS) == LDPC_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(LDPC-*): ERROR, LDPCFecSession::InitSession failed\n"))
			mcl_exit(-1);
		}
#ifdef DEBUG
		if (mclcb->get_verbosity() >= 2) {
			blk->ldpc_ses->SetVerbosity(1);
		}
#endif
		if (!(blk->callback_ctxt = (ldpc_callback_context_t*)malloc(sizeof(ldpc_callback_context_t)))) {
			goto no_memory;
		}
		blk->callback_ctxt->mclcb = mclcb;
		blk->callback_ctxt->block = blk;
		if (blk->ldpc_ses->SetCallbackFunctions(
			this->decoded_pkt_callback,
			this->alloc_tmp_buffer_callback,
			this->get_data_callback,
			this->get_data_ptr_only_callback,
			this->store_data_callback,
			this->free_pkt_callback,
			blk->callback_ctxt)
				== LDPC_ERROR) {
			PRINT_ERR((mcl_stderr,
			"mcl_fec::decode(LDPC-*): ERROR, LDPCFecSession::SetCallbackFunctions failed\n"))
			mcl_exit(-1);
		}
		if (!(blk->pkt_canvas = (void **)calloc(n, sizeof(void*)))) {
			goto no_memory;
		}

	}
	/*
	 * now decode...
	 */
#ifdef VIRTUAL_RX_MEM
	if (blk->ldpc_ses->DecodeFecStep(blk->pkt_canvas, rx_du, rx_du->seq)
				 == LDPC_ERROR)
#else
	if (blk->ldpc_ses->DecodeFecStep(blk->pkt_canvas, rx_du->data, rx_du->seq)
				 == LDPC_ERROR)
#endif
	{
		PRINT_ERR((mcl_stderr,
		"mcl_fec::decode(LDPC-*): ERROR, DecodeFecStep failed\n"))
		mcl_exit(-1);
	}
	/*
	 * check if decoding is complete...
	 */
	if (blk->du_nb_in_list >= k &&
	    blk->ldpc_ses->IsDecodingComplete(blk->pkt_canvas)) {
		/*
		 * decoding is over for this block, free all FEC DUs now...
		 */
		//blk->set_rx_status(BLK_STATUS_DECODED);
		blk->rx_status = BLK_STATUS_DECODED;
		blk->ldpc_ses->EndSession();
		delete blk->ldpc_ses;
		blk->ldpc_ses = NULL;
		ASSERT(blk->pkt_canvas);
		free(blk->pkt_canvas);
		blk->pkt_canvas = NULL;
		ASSERT(blk->callback_ctxt);
		free(blk->callback_ctxt);
		blk->callback_ctxt = NULL;
		/* free all the FEC DUs now */
		//blk->remove_and_free_all_fec_dus(mclcb);
		for (rem = blk->fec_du_nb_in_list, du = blk->fec_du_head;
		     rem > 0;
		     rem--, du = ndu) {
			ndu = du->next;
			ASSERT(du->is_fec);
			if (mclcb->rx_storage.free_data(mclcb, du) == MCL_ERROR) {
				PRINT_ERR((mcl_stderr,
				"mcl_fec::decode(RSE): ERROR: rx_storage.free_data() failed\n")) 
				goto fatal_error;
			}
			free(du);
		}
		blk->fec_du_head = NULL;
		blk->fec_du_nb_in_list = 0;
		TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode(LDPC-*): ok\n"))
		return 1;
	}
	/*
	 * not finished... keep state for next call to this function
	 */
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode(LDPC-*): not finished\n"))
	return 0;
	
no_memory:
	PRINT_ERR((mcl_stderr, "mcl_fec::decode(LDPC-*): ERROR, no memory"))
fatal_error:
	mcl_exit(-1);
	return -1;	/* unreachable; avoid a compiler warning */
}


/**
 * This callback function will be called each time a packet
 * is decoded by the DecodeFecStep() function.
 * => See header file for more informations.
 */
void *
mcl_fec::decoded_pkt_callback  (void	*context,
				INT32	size,
				INT32	pkt_seq)
{
	mcl_cb		*mclcb;		// associated mclcb
	block_t		*blk;		// associated block
	mcl_rx_pkt	*pkt;		// packet buffer
	du_t		*du;		// DU associated to pkt to create
	UINT32	du_len;			// full-sized DU length
	UINT32	last_du_len;		// last DU (true) length
	int		err;

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	blk   = ((ldpc_callback_context_t*)context)->block;
	ASSERT(blk);
	TRACELVL(5, (mcl_stdout, "-> mcl_fec::decoded_pkt_callback:\n"))

	pkt = new mcl_rx_pkt (size);
	ASSERT(pkt);
	pkt->pkt_len = size;

	//du = new mcl_du;
	du = CreateDU(mclcb);
	/* next = prev = NULL */
	du->block	= blk;
	du->seq		= pkt_seq;
	/*
	 * compute the size of a full-sized DU and of the last DU
	 * which may be shorter
	 */ 
	//du_len = mclcb->get_payload_size();
	//du_len = mclcb->payload_size;
	du_len=blk->adu->symbol_len;
	if (pkt_seq == (int)blk->k - 1) {
		/* last packet of the block, may be shorter */
		last_du_len = (blk->k > 1) ? blk->len % du_len : blk->len;
		if (last_du_len == 0)
			last_du_len = du_len;	/* multiple */
		du->len	= last_du_len;
	} else {
		/* full sized packet */
		du->len	= du_len;
	}
	ASSERT(pkt_seq < (int)blk->k);	/* make sure it's a source packet */
	du->is_fec = false;
	//du->set_next(NULL);
	//du->set_prev(NULL);
	du->data = pkt->get_buf();
	du->pkt = pkt;
	/*
	 * and insert it in the data DU list
	 */
	err = InsertDU(mclcb, du, &(blk->du_head));
	ASSERT(err == 1);
	blk->du_nb_in_list++;
	blk->adu->recvd_src_data += du->len;
	
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::decoded_pkt_callback: created aseq=%d/bseq=%d/dseq=%d  %s\n",
		blk->adu->seq, blk->seq, du->seq,
		(du->is_fec ? "FEC" : "DATA")))
	return (du);
}


/**
 * This callback function will be called each time the
 * DecodeFecStep() function needs to allocate a buffer.
 */
void *
mcl_fec::alloc_tmp_buffer_callback (void	*context,
				    int		size)
{
	mcl_cb		*mclcb;		// associated mclcb
	mcl_rx_pkt	*pkt;		// buffer
	du_t		*du;		// DU associated to buffer to create

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	TRACELVL(5, (mcl_stdout, "-> mcl_fec::alloc_tmp_buffer_callback:\n"))

	pkt = new mcl_rx_pkt (size);
	ASSERT(pkt);
	pkt->pkt_len = size;
	// we need a DU descriptor, even for temporary buffers,
	// since this is the data type used for LDPC codec/MCLv3
	// interface.
	//du = new mcl_du;
	du = CreateDU(mclcb);
	du->len = size;
	du->data = pkt->get_buf();
	du->pkt = pkt;

	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::alloc_tmp_buffer_callback: du->data=x%x, len=%d\n",
		(INT32)du->data, du->len))
	return ((void*)du);
}


void *
mcl_fec::get_data_callback (void	*context,
			    void	*pkt_du)
{
#ifdef VIRTUAL_RX_MEM
	mcl_cb		*mclcb;		// associated mclcb

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	/*
	 * retrieve data buffer that may be stored only on disk!!!
	 */
	if (mclcb->rx_storage.get_data(mclcb, (du_t*)pkt_du) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::get_data_callback: ERROR: get_data() failed\n"))
		mcl_exit(-1);
	}
	TRACELVL(5, (mcl_stdout,
		"   mcl_fec::get_data_callback: pkt_du->data=x%x, len=%d\n",
		(INT32)((du_t*)pkt_du)->data, ((du_t*)pkt_du)->len))
#endif
	return ((void*)((du_t*)pkt_du)->data);
}


void *
mcl_fec::get_data_ptr_only_callback (void	*context,
				     void	*pkt_du)
{
#ifdef VIRTUAL_RX_MEM
	mcl_cb		*mclcb;		// associated mclcb

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	TRACELVL(5, (mcl_stdout,
		"   mcl_fec::get_data_ptr_only_callback: pkt_du->data=x%x, len=%d\n",
		(INT32)((du_t*)pkt_du)->data, ((du_t*)pkt_du)->len))
	ASSERT(((du_t*)pkt_du)->data);
#endif
	return ((void*)((du_t*)pkt_du)->data);
}


/**
 * This callback function will be called each time data must
 * be stored permanently.
 * => See header file for more informations.
 */
ldpc_error_status
mcl_fec::store_data_callback (void	*context,
			      void	*pkt_du)
{
	mcl_cb		*mclcb;		// associated mclcb

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	TRACELVL(5, (mcl_stdout,
		"-> mcl_fec::store_data_callback: pkt_du->len=%d\n",
		((du_t*)pkt_du)->len))
	/*
	 * store data buffer either in vrfile (when desired and needed)
	 * or in physical memory, whichever is the most appropriate.
	 */
	if (mclcb->rx_storage.store_data(mclcb, (du_t*)pkt_du) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::store_data_callback: ERROR: store_data() failed\n"))
		mcl_exit(-1);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::store_data_callback:\n"))
	return LDPC_OK;
}


ldpc_error_status
mcl_fec::free_pkt_callback (void	*context,
			    void	*pkt_du)
{
	mcl_cb		*mclcb;		// associated mclcb

	ASSERT(context);
	mclcb = ((ldpc_callback_context_t*)context)->mclcb;
	ASSERT(mclcb);
	TRACELVL(5, (mcl_stdout, "-> mcl_fec::free_pkt_callback:\n"))
	if (mclcb->rx_storage.free_data(mclcb, (du_t*)pkt_du) == MCL_ERROR) {
		return LDPC_ERROR;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::free_pkt_callback:\n"))
	return LDPC_OK;
}

#endif /* LDPC_FEC */

