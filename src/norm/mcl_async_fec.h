/* $Id: mcl_async_fec.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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


/****** Two private classes only used by mcl_fec.cpp (private members) ********/


/**
 * FEC job class.
 * Used by the sender to ask for FEC encoding of a given block.
 * This job is enqueued on the FEC job list and encoding will be performed
 * by the FEC encoding thread.
 */
class mcl_fec_job {

public:
	/****** Public Members ************************************************/
	mcl_fec_job (mcl_block	*blk,
		     INT32	start_index,
		     INT32	fec_nb);

	~mcl_fec_job ();

	/****** Public Attributes *********************************************/
	mcl_fec_job	*next;
	mcl_block	*block;		// block to encode
	INT32		first_fec_index;// index of first FEC symbol to create
	INT32		fec_desired;	// number of FEC symbols to produce

private:

};


/**
 * Class that provides a FEC job list.
 * There is one FEC job list per session.
 */
class mcl_fec_job_list {

public:
	/****** Public Members ************************************************/
	mcl_fec_job_list ();
	~mcl_fec_job_list ();

	/**
	 * Enqueue a FEC job.
	 * @param job FEC job
	 */
	void	enqueue (mcl_fec_job *job);
	/**
	 * Dequeue a FEC job.
	 * @return FEC job if there is one, NULL if nothing to do
	 */
	mcl_fec_job	*dequeue (void);

private:
	/****** Private Members ***********************************************/
	/** Get the list lock. */
	void		lock (void);

	/** Release the list lock. */
	void		unlock (void);

	/****** Private Attributes ********************************************/
	/** linked list of FEC jobs. */
	mcl_fec_job	*head;
	mcl_fec_job	*tail;

	/** FEC job list lock. */
	mcl_mutex_t	list_lock;
};


/****** Private FEC encoding thread class (private member) ********************/


/**
 * FEC encoding thread for a given MCL session.
 */
class mcl_fec_thread {
public:
	/****** Public Members ************************************************/
	mcl_fec_thread ();
	~mcl_fec_thread ();	

	/**
	 * Starts the FEC encoding service.
	 * Exits in case of error.
	 */
	void		start (mcl_cb	*const mclcb);

	/**
	 * Return the thread id.
	 * Required to enable a safe thread destruction.
	 */
	mcl_thread_t	get_fec_thread_id ();

	/****** Public Attributes *********************************************/
  
private:
	/****** Private Members ***********************************************/

	/**
	 * Thread dedicated to FEC encoding.
	 * This thread retrieves encoding jobs from the fec_job_list,
	 * performs encoding by calling the fec_encoding_func, and
	 * enqueues FEC packets produced in the tx_window.
	 * Needs to be static since pthread_create() cannot be used with
	 * non-static member functions.
	 * @param arg	mclcb session pointer
	 */
	static void	*fec_thread (void	*arg);

	/****** Private Attributes ********************************************/

	/** Idf of the FEC encoding thread. */
	mcl_thread_t	fec_thread_id;	

};


/****** This is the public class **********************************************/


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
	 *			MCL_FEC_CODE_NULL, ...RSE, ...LDGM, ...LDPC.
	 *			(see mcl_lib_api_alc.h). Default is NULL.
	 */
	void	initialize (const INT32 new_code = 0);

	/**
	 * Return the maximum block size (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used.
	 * We have: max_k <= max_n (no FEC DU if equal).
	 */
	INT32	get_max_k () const;

	/**
	 * Return the maximum block size (in number of DUs).
	 * This parameter is set at mcl_fec service initialization
	 * depending on the FEC code used.
	 */
	INT32	get_max_n () const;

	/**
	 * Set the effective block size to use (in number of DUs).
	 * 0 < k <= max_k made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 */
	void	set_k (const INT32 k);

	/**
	 * Return the effective block size used currently (in number of DUs).
	 */ 
	INT32	get_k (void) const;

	/**
	 * Set the effective n to use (in number of DUs).
	 * 0 < n <= max_n made possible by the code.
	 * If k == n, then no FEC DU will be produced.
	 */
	void	set_n (const INT32 n);

	/**
	 * Return the effective n parameter used currently (in number of DUs).
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
	 * Set the maximum FEC ratio, defined as n/k.
	 * Calling this function modifies the cur_k/max_k accordingly.
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	set_max_fec_ratio (mcl_cb	*const mclcb,
						   float	max_fec_ratio);

	/**
	 * Return the maximum FEC ratio, defined as n/k.
	 */
	float	get_max_fec_ratio () const;

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
	bool	initialized;	/** true when encoding thread is started. */

	INT32	fec_codec;	/**
				 * Codec to use for this session.
				 * Can be one of MCL_FEC_CODE_NULL, ...RSE,
				 * ...LDGM, ...LDPC.
				 */
	INT32	max_k[MCL_FEC_CODE_MAX_NB]; /** Max k value. FEC dependant */
	INT32	max_n[MCL_FEC_CODE_MAX_NB]; /** Max n value. FEC dependant */
	INT32	cur_k[MCL_FEC_CODE_MAX_NB]; /** k value currently in use. */
	INT32	cur_n[MCL_FEC_CODE_MAX_NB]; /** n value currently in use. */
	float	max_fratio;	/** Maximum FEC ratio defined as n/k. */

#if 0
	INT32	max_k;		/** max value; FEC code dependant... */
	INT32	max_n;		/** max value; FEC code dependant... */
	INT32	cur_k;		/** k value currently in use. */
	INT32	cur_n;		/** n value currently in use. */
#endif

	/**
	 * FEC job list.
	 */
	mcl_fec_job_list	fec_job_list;

	/**
	 * FEC thread.
	 */
	mcl_fec_thread		fec_encoding_thread;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline void
mcl_fec_job_list::lock (void)
{
	mcl_lock (&(this->list_lock));
}

inline void
mcl_fec_job_list::unlock (void)
{
	mcl_unlock (&(this->list_lock));
}

inline mcl_thread_t
mcl_fec_thread::get_fec_thread_id ()
{
	return this->fec_thread_id;
}

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
mcl_fec::get_max_fec_ratio () const
{
	return this->max_fratio;
}

inline mcl_thread_t
mcl_fec::get_fec_encoding_thread_id ()
{
	return this->fec_encoding_thread.get_fec_thread_id();
}

inline INT32
mcl_fec::get_pending_fec_creation_req (mcl_block	*blk)
{
	return (blk->pending_fec_creation_req);
}

inline void
mcl_fec::incr_pending_fec_creation_req (mcl_block	*blk,
					INT32		fec_to_create)
{
	blk->pending_fec_creation_req += fec_to_create;
}

inline void
mcl_fec::decr_pending_fec_creation_req (mcl_block	*blk,
					INT32		fec_created)
{
	blk->pending_fec_creation_req -= fec_created;
}

#endif // MCL_FEC_H

