/* $Id: mcl_fec.h,v 1.16 2005/05/17 12:36:57 roca Exp $ */
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

#ifndef MCL_FEC_H  /* { */
#define MCL_FEC_H


/**
 * The following defines are specifying the identifiers used by
 * the ALC/LCT protocol. See RFC 3452 for further details.
 */

/**
 * FEC Encoding ID (RFC 3452).
 * It determines the FTI format and is carried in the LCT codepoint
 * header field.
 */
#define FEC_ENCODING_ID_NO_FEC				0
#define FEC_ENCODING_ID_SMALL_LARGE_EXP_FEC		128
#define FEC_ENCODING_ID_SMALL_SYSTEMATIC_FEC		129
// Parity Check Matrix based FEC codes.
// see the draft-peltotalo-rmt-bb-fec-xor-pcm-rs-XX.txt I-D...
#define FEC_ENCODING_ID_PCM_FEC				132


#ifdef LDPC_FEC
/**
 * context used as an opaque parameter to decoded_pkt_callback.
 */
typedef struct ldpc_callback_context {
	class mcl_cb	*mclcb;
	block_t		*block;
} ldpc_callback_context_t;
#endif


/**
 * Main FEC class.
 * Shared by the various FEC codecs.
 */
class mcl_fec {

public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 * Initializes all the supported FEC code parameters.
	 * Initializes the default FEC code.
	 */
	mcl_fec ();

	~mcl_fec ();

	/**
	 * Initializes the FEC class for all supported FEC codes:
	 *	MCL_FEC_SCHEME_NULL, ...RSE, ...LDGM, ...LDPC.
	 *	(see mcl_lib_api_alc.h).
	 * Initializes the default constructor.
	 */
	void		initialize (void);

	/**
	 * Return the maximum block size (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used, and is READ-ONLY.
	 * We have: max_k <= max_n (no FEC DU if equal).
	 * @return	maximum k.
	 */
	INT32		get_max_k () const;

	/**
	 * Return the maximum n value for a block (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used, and is READ-ONLY.
	 * @return	maximum n.
	 */
	INT32		get_max_n () const;

	/**
	 * Set the effective block size to use (in number of DUs).
	 * 0 < k <= max_k made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 * @param k	new k value.
	 */
	void		set_k (const INT32 k);

	/**
	 * Return the effective block size used currently (in number of DUs).
	 * @return	effective k value to use.
	 */ 
	INT32		get_k (void) const;

	/**
	 * Set the effective n value to use (in number of DUs).
	 * 0 < n <= max_n made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 * @param n	new n value to use.
	 */
	void		set_n (const INT32 n);

	/**
	 * Return the effective n parameter used currently (in number of DUs).
	 * @return	effective n value.
	 */ 
	INT32		get_n (void) const;

	/**
	 * Set the FEC code to be used.
	 * This function is sender specific (a receiver gets the FEC
	 * codec information info from the various incoming packet fields).
	 * This function only changes the FEC code to use from now on,
	 * without modifying anything else. It assumes that the FEC
	 * codec specific parameters have already been initialized.
	 * @param mclcb
	 * @param new_code	FEC code to use from now on, in:
	 *			{MCL_FEC_SCHEME_NULL; MCL_FEC_SCHEME_RSE;
	 *			 MCL_FEC_SCHEME_LDGM; MCL_FEC_SCHEME_LDPC}
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_fec_code (class mcl_cb	*const mclcb,
					      const INT32	new_code);

	/**
	 * Return the FEC code corresponding to the current FEC codec used.
	 * @return		FEC scheme value in:
	 *			{MCL_FEC_SCHEME_NULL; MCL_FEC_SCHEME_RSE;
	 *			 MCL_FEC_SCHEME_LDGM; MCL_FEC_SCHEME_LDPC}
	 */
	mcl_fec_scheme	get_fec_code () const;

 	/**
	 * Return the FEC code string corresponding to the current FEC codec
	 * used.
	 * WARNING: non re-entrant, the string is statically allocated...
	 * @return		string with the name of the FEC code.
	 */
	char*		get_fec_code_string () const;

	/**
	 * Set the current FEC ratio, defined as cur_n/cur_k, and update
	 * either cur_n or cur_k, depending on the FEC codec.
	 * Note that this ratio only applies to the current FEC codec, not
	 * to others, so set the FEC codec first before setting the FEC ratio!
	 * With RSE, from the cur_n value, it defines the new cur_k value,
	 * and checks it is reasonable (e.g. >= 1).
	 * With LDPC, from the cur_k value, it defines the new cur_n value,
	 * and checks it is compatible with the max_n value.
	 * This is required by the fact that the n parameter is not carried
	 * by the EXT_FTI associated to FEC encoding ID 128 and 130 (i.e.	
	 * used by NULL and RSE), unlike our private FEC encoding ID 140!
	 * It means that the appropriate FEC ratio must be set by both the
	 * sender AND receiver with FEC encoding IDs 128 and 130, but only
	 * at the sender with our FEC encoding ID 140.
	 * The above settings for cur_k and cur_n are only valid for blocks
	 * of maximum size. Otherwise, with smalle blocks, the n value
	 * associated to k < cur_k is calculated from the cur_n/cur_k ratio.
	 *
	 * @param mclcb
	 * @param new_fec_ratio	desired FEC ratio
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 *			If the new n or k value is invalid, then
	 *			an error is returned.
	 */
	mcl_error_status	set_fec_ratio (class mcl_cb	*const mclcb,
					       double		new_fec_ratio);

	/**
	 * Return the current FEC ratio, defined as cur_n/cur_k.
	 */
	double		get_fec_ratio () const;

	/**
	 * Converts from FEC Encoding ID/FEC Instance ID to FEC scheme.
	 * @param encid		FEC Encoding ID
	 * @param instid	FEC Instance ID
	 * @param scheme	pointer to FEC scheme variable to store the
	 *			result.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	enc_inst_to_scheme (
						class mcl_cb	*const mclcb,
						UINT8		encid,
						UINT8		instid,
						mcl_fec_scheme	*scheme);

	/**
	 * Converts from FEC scheme to FEC Encoding ID/FEC Instance ID.
	 * @param scheme	FEC scheme
	 * @param encid		pointer to FEC Encoding ID variable to store
	 *			the result.
	 * @param instid	pointer to FEC Instance ID variable to store
	 *			the result.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	scheme_to_enc_inst (
						class mcl_cb	*const mclcb,
						mcl_fec_scheme	scheme,
						UINT8		*encid,
						UINT8		*instid);

#if 0
	/**
	 * Return the number of available FEC packets (ie that have been
	 * created) that have not yet been sent.
	 * @param mclcb
	 * @return
	 */
	INT32		get_nb_fresh_fec_pkts (mcl_cb	*const mclcb,
						mcl_block	*blk);
#endif

	/**
	 * Return the number of FEC packets that can still be created for this
	 * block.
	 * @param mclcb
	 * @param blk		block
	 * @return		Number of FEC packets that can still be
	 * 			created for this block.
	 */
	INT32	get_rem_nb_fec_pkts_to_create (class mcl_cb	*const mclcb,
						block_t		*blk);

#ifdef FEC
	/**
	 * Encode the block and create the appropriate number of new FEC
	 * packets.
	 * The FEC packets are inserted in the block FEC list. (XXX: not yet)
	 * The number of FEC packets created is automatically determined
	 * by the block size and scheduling session.
	 * @param mclcb
	 * @param blk		block for which FEC packets must be created
	 * @return		< 0 if error, the number of FEC symbols
	 *			created if ok
	 */
	INT32		encode (class mcl_cb	*const mclcb,
				block_t		*blk);
#endif

#ifdef RSE_FEC
	/**
	 * Decode the block immediately.
	 * This function is for Reed-Solomon and similar FEC codes who,
	 * thanks to their MDS feature, decode blocks as a whole, as soon
	 * as enough (i.e. blk->k) packets have been received.
	 * @param mclcb
	 * @param blk		block for which decoding must be done
	 * @return		< 0 if error, the number of FEC symbols
	 *			created if ok
	 */
	INT32		decode (class mcl_cb	*const mclcb,
				block_t		*blk);

#endif /* RSE_FEC */

#ifdef LDPC_FEC
	/**
	 * Progress in the block decoding with the given packet.
	 * This function is for LDPC and similar FEC codes who use an
	 * iterative decoding approach.
	 * @param mclcb
	 * @param rx_du		DU for the packet just received
	 * @return		< 0 if error, the number of FEC symbols
	 *			created if ok
	 */
	INT32		decode (class mcl_cb	*const mclcb,
				du_t		*rx_du);

	/**
	 * Test if decoding is completed for this block.
	 * @param blk		block concerned
	 * @return		true if completed, false if there are
	 *			missing source packets.
	 */
	bool		is_decoding_complete (block_t	*blk);
#endif /* LDPC_FEC */


	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/

	/**
	 * Set the current FEC ratio, defined as cur_n/cur_k, from n, and
	 * update cur_k.
	 * @param mclcb
	 * @param new_fec_ratio	desired FEC ratio
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 *			If the new k value is invalid, then
	 *			an error is returned.
	 */
	mcl_error_status	set_fec_ratio_from_n
					(class mcl_cb	*const mclcb,
					 double		new_fec_ratio);

	/**
	 * Set the current FEC ratio, defined as cur_n/cur_k, from k, and
	 * update cur_n.
	 * @param mclcb
	 * @param new_fec_ratio	desired FEC ratio
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 *			If the new n value is invalid, then
	 *			an error is returned.
	 */
	mcl_error_status	set_fec_ratio_from_k
					(class mcl_cb	*const mclcb,
					 double		new_fec_ratio);

	/**
	 * Determine the number of FEC packets required for this block.
	 * Depends on the block size (k) and the kind of scheduling used
	 * for this session.
	 * Only used by a sender.
	 * @param mclcb
	 * @param k		k parameter for this block
	 * @return		n parameter for this block
	 */
	INT32	compute_n_for_this_block (class mcl_cb	*const mclcb,
					  INT32		k);

#ifdef LDPC_FEC
	/**
	 * This callback function will be called each time a packet
	 * is decoded by the DecodeFecStep() function.
	 * Create the associated DU, and insert the packet in the rx list.
	 * Allocate the buffer, using the VRM service if applicable.
	 * This function is STATIC to enable pointer-to-member-function.
	 * @param pkt_buf	pointer to the packet buffer
	 * @param pkt_seq	sequence number of packet in block
	 * 			({0..n-1} range). Can be used to
	 * 			differentiate source and FEC packets.
	 * @param context	pointer to an ldpc_callback_context_t
	 *			structure.
	 */
	static void	*decoded_pkt_callback (void	*context,
					       INT32	size,
					       INT32	pkt_seq);

	/**
	 * This callback function will be called each time the
	 * DecodeFecStep() function needs to allocate a buffer.
	 * Allocate the buffer, using the VRM service if applicable,
	 * and return the buffer. This function replaces malloc().
	 * This function is STATIC to enable pointer-to-member-function.
	 * @param context	pointer to an ldpc_callback_context_t
	 *			structure.
	 * @param size		buffer size to allocate.
	 * @return		pointer to the buffer allocated, or NULL
	 *			in case of error.
	 */
	static void	*alloc_tmp_buffer_callback (void	*context,
						    int		size);

	/**
	 * This callback function will be called each time an
	 * LDPC codec function needs to access a buffer associated
	 * to a packet.
	 */
	static void	*get_data_callback (void	*context,
					    void	*pkt_du);

	/**
	 * Same as get_data_callback, except that no check is made
	 * concerning the availability of data.
	 */
	static void	*get_data_ptr_only_callback (void	*context,
						     void	*pkt_du);

	/**
	 * This callback function will be called each time an
	 * LDPC codec function needs to make sure a buffer associated
	 * to a packet is stored permanently.
	 */
	static ldpc_error_status
			store_data_callback (void	*context,
					     void	*pkt_du);

	/**
	 * This callback function will be called each time an
	 * LDPC codec function needs to free a packet/buffer previously
	 * allocated with decoded_pkt_callback or alloc_tmp_buffer_callback.
	 */
	static ldpc_error_status
			free_pkt_callback (void	*context,
					   void	*pkt_du);


#endif /* LDPC_FEC */


	/****** Private Attributes ********************************************/

	mcl_fec_scheme	fec_codec;
				 /* Codec to use for this session.
				  * Can be one of MCL_FEC_SCHEME_NULL, ...RSE,
				  * ...LDGM, ...LDPC.
				  */

	/* max_X are _statically_ defined, at initialization */
	INT32	max_k[MCL_FEC_SCHEME_NB]; /** Max k value. FEC dependant */
	INT32	max_n[MCL_FEC_SCHEME_NB]; /** Max n value. FEC dependant */
	/* cur_X are initialized with default values, but they can be
	 * overriden latter on by the user */
	INT32	cur_k[MCL_FEC_SCHEME_NB]; /** k value currently in use. */
	INT32	cur_n[MCL_FEC_SCHEME_NB]; /** n value currently in use. */
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline INT32
mcl_fec::get_max_k () const
{
	return this->max_k[this->fec_codec];
}

inline INT32
mcl_fec::get_max_n () const
{
	return this->max_n[this->fec_codec];
}

inline INT32
mcl_fec::get_k (void) const
{
	return this->cur_k[this->fec_codec];
}

inline void
mcl_fec::set_k (const INT32 k)
{
	ASSERT(k <= max_k[this->fec_codec]);
	ASSERT(k > 0);
	ASSERT(k <= cur_n[this->fec_codec]);
	this->cur_k[this->fec_codec] = k;
}

inline INT32
mcl_fec::get_n () const
{
	return this->cur_n[this->fec_codec];
}

inline void
mcl_fec::set_n (const INT32 n)
{
	ASSERT(n <= max_n[this->fec_codec]);
	ASSERT(n > 0);
	ASSERT(cur_k[this->fec_codec] <= n);
	this->cur_n[this->fec_codec] = n;
}

inline mcl_fec_scheme
mcl_fec::get_fec_code () const
{
	/*
	 * WARNING: assumes that lct_codepoint values are the same
	 * as the MCL_FEC_CODE_XXX values!
	 */
	return (mcl_fec_scheme) this->fec_codec;
}

inline double
mcl_fec::get_fec_ratio () const
{
	return ((double)this->get_n() / (double)this->get_k());
}

#ifdef LDPC_FEC
inline bool
mcl_fec::is_decoding_complete (block_t	*blk)
{
	ASSERT(blk->ldpc_ses);
	ASSERT(blk->pkt_canvas);
	return (blk->ldpc_ses->IsDecodingComplete(blk->pkt_canvas));
}
#endif

#endif /* }  MCL_FEC_H */
