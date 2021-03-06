/* $Id: mcl_fec.h,v 1.5 2004/08/03 06:35:56 roca Exp $ */
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

#ifndef MCL_FEC_H
#define MCL_FEC_H

class mcl_block;

/*
 * Simplified synchronous FEC class.
 * Encoding is performed in the current thread rather than in a dedicated
 * thread (see mcl_async_thread.h/cpp).
 */


/**
 * Main FEC class.
 */
class mcl_fec {
friend class mcl_fec_thread;	// required for fec_thread to access
				// fec_encoding_func() and fec_job_list

public:
	/****** Public Members ************************************************/
	mcl_fec ();
	~mcl_fec ();

	/**
	 * Initializes the FEC class for a given FEC code.
	 * Replaces the default constructor.
	 * @param new_code	FEC code to use. Can be one of
	 *			MCL_FEC_SCHEME_NULL, ...RSE, ...LDGM, ...LDPC.
	 *			(see mcl_lib_api_alc.h). Default is NULL.
	 */
	void	initialize (const INT32 new_code = 0);

	/**
	 * Return the maximum block size (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used, and is READ-ONLY.
	 * We have: max_k <= max_n (no FEC DU if equal).
	 * @return	maximum k.
	 */
	INT32	get_max_k () const;

	/**
	 * Return the maximum n value for a block (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used, and is READ-ONLY.
	 * @return	maximum n.
	 */
	INT32	get_max_n () const;

	/**
	 * Set the effective block size to use (in number of DUs).
	 * 0 < k <= max_k made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 * @param k	new k value.
	 */
	void	set_k (const INT32 k);

	/**
	 * Return the effective block size used currently (in number of DUs).
	 * @return	effective k value to use.
	 */ 
	INT32	get_k (void) const;

	/**
	 * Set the effective n value to use (in number of DUs).
	 * 0 < n <= max_n made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 * @param n	new n value to use.
	 */
	void	set_n (const INT32 n);

	/**
	 * Return the effective n parameter used currently (in number of DUs).
	 * @return	effective n value.
	 */ 
	INT32	get_n (void) const;

	/**
	 * Set the FEC code to be used.
	 * This function is sender specific (a receiver gets the FEC
	 * codec information info from the various incoming packet fields).
	 * This function only changes the FEC code to use from now on,
	 * without modifying anything else. It assumes that the FEC
	 * codec specific parameters have already been initialized.
	 * @param new_code	FEC code to use from now on...
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_fec_code (class mcl_cb	*const mclcb,
					      const INT32	new_code);

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

	mcl_error_status	set_fec_ratio (mcl_cb	*const mclcb,
						float	new_fec_ratio);

	/**
	 * Return the current FEC ratio, defined as n/k.
	 */
	float	get_fec_ratio () const;

	/**
	 * Return the number of available FEC packets (ie that have been
	 * created) that have not yet been sent.
	 */
	INT32	get_nb_fresh_fec_pkts (mcl_cb	*const mclcb,
					mcl_block	*blk);

	/**
	 * Return the number of FEC packets that can still be created for this
	 * block.
	 * Does NOT take into account the number of pending FEC creation
	 * requests.
	 */
	INT32	get_rem_nb_fec_pkts_to_create (mcl_cb	*const mclcb,
						mcl_block	*blk);

	/**
	 * Return the number of pending FEC creation requests for this  block.
	 * This is the sum of packets to create over all pending FEC jobs for
	 * this block.
	 * @param blk	block concerned
	 * @return	number of FEC packets that are to be created
	 */
	INT32	get_pending_fec_creation_req (mcl_block	*blk);

	/**
	 * Increments the number of pending FEC creation requests for this
	 * block.
	 * Used after the creation of a FEC job.
	 * @param blk	block concerned
	 * @param fec_to_create
	 */
	void	incr_pending_fec_creation_req (mcl_block	*blk,
						INT32		fec_to_create);

	/**
	 * Decrements the number of pending FEC creation requests for this
	 * block.
	 * Used after the completion of a FEC job.
	 * @param blk	block concerned
	 * @param fec_created
	 */
	void	decr_pending_fec_creation_req (mcl_block	*blk,
						INT32		fec_created);

	/**
	 * Encode the block and create the requested number of new FEC packets.
	 * Encoding is done asynchronously by the fec_encoding_thread
	 * and the results inserted automatically in the sender's
	 * transmission queue.
	 * @param mclcb
	 * @param blk	block for which FEC packets must be created
	 * @param fec_desired	number of FEC packets that must be created
	 * @return < 0 if error, the number of FEC symbols created if ok
	 */
	INT32	encode (mcl_cb		*const mclcb,
			mcl_block	*blk,
			INT32		fec_desired);

	/**
	 * Decode the block.
	 * Decoding is done immediately.
	 * @return < 0 if error, the number of FEC symbols created if ok
	 */
	INT32	decode (mcl_cb		*const mclcb,
			mcl_block	*blk);


	/**
	 * Return the thread id.
	 * Required to enable a safe thread destruction.
	 */
	mcl_thread_t	get_fec_encoding_thread_id ();

	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/
	/**
	 * Set the current FEC ratio, defined as cur_n/cur_k, from n, and
	 * update cur_k.
	 * @param mclcb
	 * @param new_fec_ratio	desired FEC ratio
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 *			If the new k value is invalid, then
	 *			an error is returned.
	 */
	mcl_error_status	set_fec_ratio_from_n (mcl_cb	*const mclcb,
						      float	new_fec_ratio);

	/**
	 * Set the current FEC ratio, defined as cur_n/cur_k, from k, and
	 * update cur_n.
	 * @param mclcb
	 * @param new_fec_ratio	desired FEC ratio
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 *			If the new n value is invalid, then
	 *			an error is returned.
	 */
	mcl_error_status	set_fec_ratio_from_k (mcl_cb	*const mclcb,
						      float	new_fec_ratio);

	/**
	 * Performs FEC encoding for this block.
	 * This function may be called several times for the same block (even
	 * if calling it once may be faster), for instance to satisfy
	 * additional retransmission requests.
	 * @param blk	block for which FEC packets must be created
	 * @param start_index	index of the first (inclusive) FEC packet
	 * 			to create
	 * @param fec_desired	number of FEC packets that must be created
	 * 			from start_index
	 * @returns	number of FEC packets created if ok, < 0 if error.
	 */
	INT32			fec_encoding_func (mcl_cb	*const mclcb,
						   mcl_block	*blk,
						   INT32	start_index,
						   INT32	fec_desired);


	/****** Private Attributes ********************************************/

	INT32	fec_codec;	/**
				 * Codec to use for this session.
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

#if 0
	/* fec_ratio is initialized with a default value, but it can be
	 * overriden latter on by the user */
	float	fec_ratio;		/** Current FEC ratio defined as n/k. */
#endif
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

inline float
mcl_fec::get_fec_ratio () const
{
	return ((double)this->get_n() / (double)this->get_k());
}

#endif // MCL_FEC_H

