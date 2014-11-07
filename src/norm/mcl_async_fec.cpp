/* $Id: mcl_async_fec.cpp,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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


/****** Following two classes are private and only used by mcl_fec.cpp ******/

/*
 * FEC job class.
 */
mcl_fec_job::mcl_fec_job (mcl_block	*blk,
			  INT32		start_index,
			  INT32		fec_nb)
{
	this->next = NULL;
	this->block = blk;
	this->first_fec_index = start_index;
	this->fec_desired = fec_nb;
}


mcl_fec_job::~mcl_fec_job ()
{
}


/*
 * Class that provides a FEC job list.
 */
mcl_fec_job_list::mcl_fec_job_list ()
{
	this->head = NULL;
	this->tail = NULL;
	mcl_init_lock (&(this->list_lock));
}

mcl_fec_job_list::~mcl_fec_job_list ()
{
}


/*
 * insert in tail
 */
void
mcl_fec_job_list::enqueue (mcl_fec_job *job)
{
	this->lock();
	ASSERT(job->next == NULL);
	if (this->tail) {
		this->tail->next = job;
	} else {
		ASSERT(this->head == NULL);
		this->head = job;
	}
	this->tail = job;
	this->unlock();
}


/*
 * remove from head
 */
mcl_fec_job *
mcl_fec_job_list::dequeue (void)
{
	mcl_fec_job	*job;

	this->lock();
	if (this->head) {
		job = this->head;
		this->head = job->next;
		if (this->tail == job) {
			this->tail = NULL;
			ASSERT(this->head == NULL);
		}
	} else {
		ASSERT(this->tail == NULL);
		job = NULL;
	}
	this->unlock();
	return job;
}


/****** Private FEC encoding thread class *************************************/

mcl_fec_thread::mcl_fec_thread ()
{
	this->fec_thread_id = 0;
}


mcl_fec_thread::~mcl_fec_thread ()
{
	this->fec_thread_id = 0;
}


void
mcl_fec_thread::start (mcl_cb	*const mclcb)
{
	/*
	 * create the fec thread...
	 */
#ifdef WIN32
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) mcl_fec_thread::fec_thread,
			 NULL, 0, (LPDWORD)mclcb) == NULL) {
		perror("mcl_fec_thread::start: CreateThread");
		mcl_exit(1);
	}
#else
	if (pthread_create((pthread_t*)&(this->fec_thread_id),
				NULL,
				mcl_fec_thread::fec_thread,
				(void*)mclcb) != 0) {
		perror("mcl_fec_thread::start: pthread_create");
		mcl_exit(1);
	}
#endif
}


#if 0
/**
 * Stops the FEC encoding service.
 */
void
mcl_fec_thread::stop (mcl_cb	*const mclcb)
{
#ifndef WIN32
	if (this->fec_thread_id != 0) {
		pthread_cancel(this->fec_thread_id);
		pthread_join(this->fec_thread_id, NULL);
	}
#endif
}
#endif


/**
 * FEC encoding thread.
 * Retrieve a job from the job list, perform FEC encoding, and put encoded
 * packets on the transmission waiting queue
 */
void *
mcl_fec_thread::fec_thread (void	*arg)
{
	mcl_cb		*mclcb = (mcl_cb*)arg;
	mcl_fec_job	*job;
	mcl_block	*blk;
	mcl_du		*du;
	INT32		i;
	INT32		fec_created;

	TRACELVL(5, (mcl_stdout, "-> mcl_fec_thread::fec_thread:\n"))
#ifndef WIN32
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/*
	 * cancellation is deferred till next check point , i.e. points
	 * in code where we know everything is in a stable state
	 */
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#endif
	while (1) {
#ifndef WIN32
		pthread_testcancel(); // check when thread has no lock
#endif
		while ((job = mclcb->fec.fec_job_list.dequeue()) == NULL) {
			mcl_usleep(DFLT_POLLING_PERIOD);
#ifndef WIN32
			pthread_testcancel(); // check when thread has no lock
#endif

		}
		/*
		 * got a new job, now create the desired fec...
		 */
		blk = job->block;
		TRACELVL(5, (mcl_stdout,
			"   mcl_fec_thread::fec_thread: create %d fec for blk %d/%d\n",
			job->fec_desired, blk->adu->get_seq(), blk->seq))
		/*
		 * Produce FEC packets.
		 * XXX: a bit dangerous...
		 * Block object modified without being locked for this mclcb!
		 * This is required though to enable an asynchronous fec
		 * encoding...
		 * There is a risk if the session is destroyed in the meantime,
		 * or the adu free'ed...
		 */
		fec_created = mclcb->fec.fec_encoding_func(mclcb, blk,
							job->first_fec_index,
							job->fec_desired);
#ifndef WIN32
		pthread_testcancel(); // check when thread has no lock
#endif
		/*
		 * and now get the session lock...
		 */
		mclcb->lock();
		/*
		 * no longer pending.
		 * NB: decrement by fec_desired and not fec_created since
		 * no additional fec will be created for this job, even
		 * if fec_created < fec_desired...
		 */
		mclcb->fec.decr_pending_fec_creation_req(blk, job->fec_desired);
		//if (blk->get_fec_du_nb_in_list() < job->fec_desired)
		if (fec_created < job->fec_desired) {
			/* failed, the encoder didn't produce the desired FEC */
			PRINT_ERR((mcl_stderr,
				"mcl_fec_thread::fec_thread: ERROR, mcl_fec_encode failed: wanted %d, got %d\n",
				job->fec_desired, fec_created))
#ifdef DEBUG
			mcl_exit(-1);
#endif
		}
		/*
		 * register each FEC packet created in the tx window.
		 * Must be done starting by the end of FEC list as FEC DUs
		 * may have already been produced.
		 */
		for (du = blk->get_fec_du_tail(), i = fec_created;
		     i > 0; i--, du = du->get_prev()) {
			ASSERT(du);
			mclcb->tx_window.register_new_du(mclcb, du);
		}
		mclcb->unlock();
#ifndef WIN32
		pthread_testcancel(); // check when thread has no lock
#endif
		delete job;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_fec_thread::fec_thread:\n"))
}


/****** And this is the public class ******************************************/


#if 0
mcl_fec::mcl_fec ()
{
	this->initialized = false;
#ifdef RSE_FEC
	this->max_k = this->max_n = GF_SIZE;
#else
	// no fec codec used, so use default conservative values...
	this->max_k = this->max_n = 255;
#endif
	this->cur_n = this->max_n;
	this->cur_k = this->max_n >> 1;		// default: half data, half fec
}


mcl_fec::~mcl_fec ()
{
}
#endif


mcl_fec::mcl_fec ()
{
	/* set everything to 0 */
	memset(this->max_k, 0, sizeof(this->max_k));
	memset(this->max_n, 0, sizeof(this->max_n));
	memset(this->cur_k, 0, sizeof(this->cur_k));
	memset(this->cur_n, 0, sizeof(this->cur_n));
	/*
	 * initialize the fec class for each supported FEC code.
	 * the order is significant since the last call specifies the
	 * default FEC code.
	 */
	this->initialize(MCL_FEC_CODE_NULL);
#ifdef RSE_FEC
	this->initialize(MCL_FEC_CODE_RSE);
#endif

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
mcl_fec::initialize (const INT32	new_code)
{
	switch (new_code) {
#ifdef RSE_FEC
	case MCL_FEC_CODE_RSE:
		this->fec_codec = MCL_FEC_CODE_RSE;
		ASSERT(RSE_MAX_K <= RSE_MAX_N);
		ASSERT(RSE_MAX_N <= GF_SIZE);
		this->max_k[MCL_FEC_CODE_RSE] = RSE_MAX_K;
		this->max_n[MCL_FEC_CODE_RSE] = RSE_MAX_N;
		this->cur_k[MCL_FEC_CODE_RSE] = RSE_MAX_K;
		this->cur_n[MCL_FEC_CODE_RSE] = min(2 * RSE_MAX_K, RSE_MAX_N);
				// default: half data, half fec
		break;
#endif
	case MCL_FEC_CODE_NULL:
		// no fec codec used, so use default conservative values...
		this->fec_codec = MCL_FEC_CODE_NULL;
		this->max_k[MCL_FEC_CODE_NULL] = 255;
		this->max_n[MCL_FEC_CODE_NULL] = 255;
		this->cur_k[MCL_FEC_CODE_NULL] = 255;
		this->cur_n[MCL_FEC_CODE_NULL] = 255;
		break;
	default:
		PRINT_ERR((mcl_stderr,
			"mcl_fec::initialize: ERROR, called with unsupported FEC code %d\n",
			new_code))
		mcl_exit(-1);
	}
	this->max_fratio = this->cur_n[new_code] / this->cur_k[new_code];
	/*
	 * check it is coherent with the MAX_FEC_RATIO constant
	 * defined in mcl_profile.h
	 */
	if (this->max_fratio > MAX_FEC_RATIO) {
		PRINT_ERR((mcl_stderr,
			"mcl_fec::initialize: ERROR, the calculated max_fratio(%f) is larger than MAX_FEC_RATIO (%f) for code %d\nCheck and correct the various settings in mcl_profile.h accordingly...\n",
			this->max_fratio, MAX_FEC_RATIO, new_code))
		mcl_exit(-1);
	}
}


/**
 * Set the FEC code to be used.
 * This function is sender specific (a receiver gets the FEC
 * codec information info from the various incoming packet fields).
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::set_fec_code (mcl_cb		*const mclcb,
		       const INT32	new_code)
{
	if (new_code == MCL_FEC_CODE_NULL) {
		this->fec_codec = MCL_FEC_CODE_NULL;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use NO FEC\n"))
		return MCL_OK;
	}
#ifdef RSE_FEC
	else if (new_code == MCL_FEC_CODE_RSE) {
		this->fec_codec = MCL_FEC_CODE_RSE;
		ASSERT(get_max_k() > 0); /* must be true if initialized */
		TRACELVL(5, (mcl_stdout, "   mcl_fec::set_fec_code: use RSE\n"))
		return MCL_OK;
	}
#endif
	PRINT_ERR((mcl_stderr,
		"mcl_fec::set_fec_code: ERROR, code %d unknown\n", new_code))
	return MCL_ERROR;
}


#if 0
/**
 * Stop the encoding thread.
 */
void
mcl_fec::stop ()
{
	if (this->initialized == true) {
		// delete the thread
		this->fec_encoding_thread.stop(mclcb);
		this->initialized = false;
	} // else nothing to do, thread not created.
	return MCL_OK;

}
#endif

#if 0
/**
 * Set the maximum FEC ratio, defined as n/k.
 * Calling this function modifies the cur_k/max_k accordingly.
 * @return Completion status (MCL_OK or MCL_ERROR).
 */
mcl_error_status
mcl_fec::set_max_fec_ratio (mcl_cb	*const mclcb,
			    float	max_fec_ratio)
{
	INT32	tmp_max_k;
	INT32	tmp_cur_k;

	if (max_fec_ratio < 1.0 || max_fec_ratio > 10.0) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, bad max_fec_ratio; got %f, should be in [1.0; 10.0]\n", max_fec_ratio))
		goto bad;
	}
	tmp_max_k = (INT32)(this->max_n / max_fec_ratio);
	tmp_cur_k = (INT32)(this->cur_n / max_fec_ratio);
	if (tmp_max_k < 1 || tmp_cur_k < 1) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, max_fec_ratio %f too large\n", max_fec_ratio))
		goto bad;
	}
	this->max_k = tmp_max_k;
	this->cur_k = tmp_cur_k;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::set_max_fec_ratio: max_k/n=(%d; %d), cur_k/n=(%d; %d)\n",
		this->max_k, this->max_n, this->cur_k, this->cur_n))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, bad max_fec_ratio; got %f, should be in [1.0; 10.0]\n", max_fec_ratio))
	return MCL_ERROR;
}
#endif

/**
 * Set the maximum FEC ratio, defined as n/k.
 * => See header file for more informations.
 */
mcl_error_status
mcl_fec::set_max_fec_ratio (mcl_cb	*const mclcb,
			    float	max_fec_ratio)
{
	INT32	tmp_max_k;
	INT32	tmp_cur_k;

	if (max_fec_ratio < 1.0 || max_fec_ratio > 10.0) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, bad max_fec_ratio; got %f, should be in [1.0; 10.0]\n", max_fec_ratio))
		goto bad;
	}
	tmp_max_k = (INT32)(this->get_max_n() / max_fec_ratio);
	tmp_cur_k = (INT32)(this->get_n() / max_fec_ratio);
	if (tmp_max_k < 1 || tmp_cur_k < 1) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, max_fec_ratio %f too large\n", max_fec_ratio))
		goto bad;
	}
	this->max_k[this->fec_codec] = tmp_max_k;
	this->set_k(tmp_cur_k);
	this->max_fratio = max_fec_ratio;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_fec::set_max_fec_ratio: max_k/n=(%d; %d), cur_k/n=(%d; %d)\n",
		this->get_max_k(), this->get_max_n(),
		this->get_k(), this->get_n()))
	return MCL_OK;

bad:
	PRINT_ERR((mcl_stderr,
		"mcl_fec::set_max_fec_ratio: ERROR, bad max_fec_ratio; got %f, should be in [1.0; 10.0]\n", max_fec_ratio))
	return MCL_ERROR;
}



/**
 * Return the number of available FEC packets (ie that have been
 * created) that have not yet been sent.
 */
INT32
mcl_fec::get_nb_fresh_fec_pkts (mcl_cb		*const mclcb,
				mcl_block	*blk)
{
	return 0;	// TODO, in the meantime assume everything has been sent
}


/**
 * Return the number of FEC packets that can still be created for this
 * block.
 * Does NOT take into account the number of pending FEC creation requests.
 */
INT32
mcl_fec::get_rem_nb_fec_pkts_to_create (mcl_cb		*const mclcb,
					mcl_block	*blk)
{
	return (this->get_n() - blk->du_nb - blk->get_fec_du_nb_in_list());
}


/**
 * Encode the block and create the requested number of new FEC packets.
 * Encoding is done asynchronously by the fec_thread
 * and the results inserted automatically in the sender's
 * transmission queue and in the block's FEC du list.
 * @return < 0 if error, the number of FEC symbols created if ok
 */
INT32
mcl_fec::encode (mcl_cb		*const mclcb,
		 mcl_block	*blk,
		 INT32		fec_desired)
{
	INT32		k;		// effective k for this block
	INT32		max_fec;	// max nb of fec for this blk
	mcl_fec_job	*job;
	INT32		start_index;	// index of 1st FEC pkt to create
	INT32		fec_to_create;

	TRACELVL(5, (mcl_stdout,
		"-> mcl_fec::encode: blk seq=%d, want %d fec\n",
		blk->seq, fec_desired))
	if (this->initialized == false) {
		this->fec_encoding_thread.start(mclcb);
		this->initialized = true;
	}
	/*
	 * Sanity checks...
	 */
	k = blk->du_nb;
	ASSERT(k <= this->get_k());
	max_fec = get_rem_nb_fec_pkts_to_create(mclcb, blk);
	if (fec_desired > max_fec || fec_desired <= 0) {
		PRINT_ERR((mcl_stderr,
			"mcl_fec::encode: ERROR, illegal nb of FEC symbols desired (got %d, should be in ]0; %d]\n",
			fec_desired, max_fec))
		return (-1);
	}
	/* do not consider the nb of pending fec creation requests */
	start_index = k + blk->get_fec_du_nb_in_list()
			+ this->get_pending_fec_creation_req(blk);
	fec_to_create = fec_desired - this->get_pending_fec_creation_req(blk);
	if (fec_to_create <= 0) {
		TRACELVL(5, (mcl_stdout, "<- mcl_fec::encode: not required, enough pending FEC creations\n"))
		return(0);
	}
	/*
	 * create a job and insert it in the fec_job list
	 */
	job = new mcl_fec_job(blk, start_index, fec_to_create);
	this->fec_job_list.enqueue(job);
	this->incr_pending_fec_creation_req(blk, fec_to_create); /* recorded */
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::encode:\n"))
	return fec_desired;
}


/**
 * Performs FEC encoding for this block.
 * This function may be called several times for the same block (even
 * if calling it once may be faster), for instance to satisfy additional
 * retransmission requests.
 * @param blk		block for which FEC packets must be created
 * @param start_index	index of the first (inclusive) FEC packet to create
 * @param fec_desired	number of FEC packets that must be created from
 * 			start_index
 * @returns		number of FEC packets created if ok, < 0 if error.
 */
INT32
mcl_fec::fec_encoding_func (mcl_cb	*mclcb,
			    mcl_block	*blk,
			    INT32	start_index,
			    INT32	fec_desired)
{
	INT32 	k;			/* k source DUs... */
					/* FEC DU seq nbs follow source DUs */
	INT32	n;			/* total of n FEC+source DUs */
	UINT32 	du_len;
	UINT32	last_du_len;
	INT32	i;
	void	*code;
	mcl_du	*fec_du;
	char	*fec_data;
	char	**src;
	mcl_du	*du;
	
	ASSERT(blk);
	TRACELVL(5, (mcl_stdout,
		"-> mcl_fec::fec_encoding_func: blk=%d, want %d fec\n",
		blk->seq, fec_desired))
	k = blk->du_nb;
	ASSERT(start_index >= k);
	ASSERT(fec_desired > 0);
	/*
	 * compute the size of a full-sized DU and of the last DU
	 * which may be shorter
	 */ 
	du_len = mclcb->get_payload_size();
	last_du_len = (k > 1) ? blk->len % du_len : blk->len;
	if (last_du_len == 0)
		last_du_len = du_len;	/* multiple */
	/*
	 * create all the required FEC DUs for this block
	 */
	n = start_index + fec_desired;
	ASSERT(n <= this->get_n());

	if (!(code = fec_new(k,n))) {
		goto no_memory;
	}
	/*
	 * no data copy except for the last du which may be shorter
	 */
	if (!(src = (char**)malloc(sizeof(char*) * n))) {
		goto no_memory;
	}
	for (i = 0, du = blk->get_du_head(); i < k - 1; i++, du++) {
		src[i] = (char*)(du->data);
	}
	if (!(src[k-1] = (char*)calloc(1, du_len))) {
		goto no_memory;
	}
	ASSERT(du->seq == (UINT32)k-1);
	ASSERT(du->len == last_du_len);
	/*
	 * now create the n-k FEC DUs
	 */
	memcpy(src[k-1], du->data, last_du_len);
	for (i = start_index; i < n; i++) {
		if (!(fec_data = (char *) malloc(du_len))) {
			goto no_memory;
		}
		fec_encode(code, (void**) src, fec_data, i, du_len);
		fec_du = new mcl_du;
		fec_du->block	= blk;
		fec_du->data 	= fec_data;
		fec_du->seq	= i;
		fec_du->len 	= du_len;
		fec_du->is_fec 	= true;
		fec_du->pkt	= NULL;	// this is how a tx DU is distinguished
					// from a rx DU!
		fec_du->set_next(NULL);
		fec_du->set_prev(NULL);
		/*
		 * Use a linked list of FEC DUs rather than a single tab as
		 * with data DUs on the sending side!
		 */
		if (blk->insert_in_fec_du_list(mclcb, fec_du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr, "mcl_fec::fec_encoding_func: ERROR, insert_in_fec_du_list() failed\n"))
//#if 0
printf("insert failed for fec_du seq=%d; start_idx=%d, fec_desired=%d\n", fec_du->seq, start_index, fec_desired);
int ii;
mcl_du	*fdu;
for (ii=0, fdu = blk->get_fec_du_head(); ii<blk->get_fec_du_nb_in_list(); ii++, fdu = fdu->get_next()){
	printf("fec_du: seq=%d\n", fdu->seq);
}
//#endif
			mcl_exit(-1);
		}
		TRACELVL(5, (mcl_stdout,
			"   mcl_fec::fec_encoding_func: created FEC seq=%d, len=%d, buf=x%x\n",
			(INT32)fec_du->seq, (INT32)fec_du->len,
			(INT32)fec_du->data))
	}
	free(src[k-1]);
	free(src);
	fec_free(code);
	TRACELVL(5, (mcl_stdout, "<- mcl_fec::fec_encoding_func: %d fec_du\n",
		n - start_index))
	return (n - start_index);

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_fec::fec_encoding_func: ERROR, no memory\n"))
	mcl_exit(-1);
	return -1;	// useless but avoids a warning
}


INT32
mcl_fec::decode (mcl_cb		*const mclcb,
		 mcl_block	*blk)
{
	INT32 	k;
	INT32 	n;
	UINT32	du_len;		/* full-sized DU length */
	UINT32	last_du_len;	/* last DU (true) length */
	INT32	i;
	u_char	**dst;		/* put rx data or fec here... */
	INT32	rx_index[GF_SIZE]; /* ... and their seq# here */
	UINT32 	rem;		/* remaining nb of DUs */
	void 	*code;
	mcl_du	*du;
	UINT32	seq;		/* next seq number expected */
#ifdef DEBUG
	char	*data;		/* data buffer */
#endif
	mcl_rx_pkt	*pkt;	/* used for data DUs created during decoding */
	u_char	*buf;		/* buffer where data/FEC DU data is copied */
	INT32	off;		/* offset */


	ASSERT(blk);
	TRACELVL(5, (mcl_stdout, "-> mcl_fec::decode: block seq=%d\n", blk->seq))
	/*
	 * check that FEC decode is indeed needed (i.e. that
	 * we need at least one FEC DU)
	 */
	k = blk->du_nb;
	if (blk->get_du_nb_in_list() >= k) {
		blk->set_rx_status(BLK_STATUS_DECODED);
		TRACELVL(5, (mcl_stdout,
			"   mcl_fec::decode: decoding not needed\n"))
		/*TRACELVL(5, ("decode: k=%d, du_in_list=%d, fec_in_list=%d\n", k, blk->get_du_nb_in_list(), blk->get_fec_du_nb_in_list()))*/
		goto free_fec_du;
	}
	/*
	 * compute the size of a full-sized DU and of the last DU
	 * which may be shorter
	 */ 
	du_len = mclcb->get_payload_size();
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
	i = 0;			/* index in rx_index[] */
	/*
	 * copy data DU to the dst array first
	 */
	for (rem = blk->get_du_nb_in_list(), du = blk->get_du_head();
	     rem > 0; i++, rem--) {
		ASSERT(du);
		/* WARNING: if the following test failed, you probably forgot */
		/* to set the same tx_profile at the source AND receiver */
		if (du->len != du_len && du->len != last_du_len) {
			PRINT_ERR((mcl_stderr, "mcl_fec::decode: ERROR: bad packet len (expected %d or %d, got %d)\nCheck transmission profiles at sender/receiver\n", 
				du_len, last_du_len, du->len))
			goto fatal_error;
		}
		if (i >= k)
			break;	/* security, eg. if there are more than k DUs */
		memcpy(dst[i], du->data, du->len);
		if (du->len < du_len) {
			/* non full-sized DU, so reset the remaining bytes */
			memset(dst[i] + du->len, 0, du_len - du->len);
		}
		rx_index[i] = du->seq;
#if 0
		PRINT_OUT((mcl_stdout, "decode: copy DATA du->seq=%d in dst[%d], rx_index[%d]=%d\n",
			du->seq, i, i, rx_index[i]))
#endif
		du = du->get_next();
	}
	/*
	 * copy FEC DUs to the dst array now
	 */
	for (rem = blk->get_fec_du_nb_in_list(), du = blk->get_fec_du_head();
	     rem > 0; i++, rem--, du = du->get_next()) {
		ASSERT(du);
		ASSERT(du->len == du_len);
		if (i >= k) {
			/* security, if there are more than k data+FEC DUs */
			break;
		}
		memcpy(dst[i], du->data, du->len);
		/* rx_index[i] = du->seq + k; */
		rx_index[i] = du->seq;
#if 0
		PRINT_OUT((mcl_stdout, "decode: copy FEC du->seq=%d in dst[%d], rx_index[%d]=%d\n",
			du->seq, i, i, rx_index[i]))
#endif
	}
	/*
	 * now decode
	 * NB: all the dst packets must be du_len long
	 */
	n = this->get_n();
	code = fec_new(k, n);
	if (fec_decode(code, (void**)dst, rx_index, du_len)) {
		PRINT_ERR((mcl_stderr, "mcl_fec::decode: ERROR, fec_decode failed\n"))
		return -1;
	}
	fec_free(code);

	/*
	 * now update the block with the rx or reconstructed DUs
	 */
	for (rem = blk->du_nb, seq = 0, du = blk->get_du_head();
	     rem > 0; rem--, seq++, du = du->get_next()) {
		if (du && du->seq == seq) {
			/* nothing to do, DU already received */
			continue;
		}
		/*
		 * else copy it from the FEC decoded matrix
		 */
		du = new mcl_du;

		du->block	= blk;
		du->seq		= seq;
		if (rem > 1)
			du->len	= du_len;
		else
			du->len	= last_du_len;
		du->is_fec 	= false;
		du->set_next(NULL);
		du->set_prev(NULL);
		/*
		 * copy/store data now
		 */
		pkt = new mcl_rx_pkt (du->len);
		pkt->pkt_len = du->len;
		du->pkt = pkt;
		du->data = pkt->get_buf();
		memcpy(du->data, dst[seq], du->len);
		/*
		 * and insert it in the data DU list
		 */
		if (blk->insert_in_du_list(mclcb, du) == MCL_ERROR) {
			PRINT_ERR((mcl_stderr, "mcl_fec::decode: ERROR, insert_in_fec_du_list() failed\n"))
			mcl_exit(-1);
		}
		TRACELVL(5, (mcl_stdout,
			"   decode: created DU seq=%d from dst[%d], len=%d, data buffer=x%x, du->data=x%x\n",
			du->seq, seq, du->len, (int)data, (int)du->data))
#if 0
		DumpBuffer(data, sizeof(char*) + du->len, (sizeof(char*) + du->len)>>2);
#endif
	}
	blk->set_rx_status(BLK_STATUS_DECODED);

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
	blk->remove_and_free_all_fec_dus(mclcb);

	TRACELVL(5, (mcl_stdout, "<- mcl_fec::decode: ok\n"))
	return 0;

no_memory:
	PRINT_ERR((mcl_stderr, "mcl_fec::decode: ERROR, no memory"))
	mcl_exit(-1);

fatal_error:
	PRINT_ERR((mcl_stderr, "mcl_fec::decode: ERROR"))
	mcl_exit(-1);
	return -1;	/* unreachable; avoid a compiler warning */
}

