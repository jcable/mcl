/* $Id: mcl_sched.cpp,v 1.12 2005/05/23 11:11:50 roca Exp $ */
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
 *	Transmission plannification for each new ADU.
 */

#include "mcl_includes.h"


/*
 * Create the sending tx planning after receiving a new ADU.
 * Called by the sending function of the lib.
 * NB: not implemented with processing speed in mind!
 */


#ifdef LCT_SCHED1
static void LCT1_UpdateTxPlanning	(mcl_cb *mclcb, adu_t *adu_start,
					adu_t *adu_end);
#endif
#ifdef LCT_SCHED2
static void LCT2_UpdateTxPlanning	(mcl_cb *mclcb, adu_t *adu_start,
					adu_t *adu_end);
#endif
#ifdef LCT_SCHED3
static void LCT3_UpdateTxPlanning	(mcl_cb *mclcb, adu_t *adu_start,
					adu_t *adu_end);
#endif


/*
 * scheduler switching table
 */
typedef void (*UpdateTxPlanning_func) (mcl_cb *mclcb, adu_t *adu_start, adu_t *adu_end);
static UpdateTxPlanning_func	sched_tab[MCL_SCHED_NB];


/****** LCT scheduling common functions ***************************************/


/*
 * switch according to the scheduler in use
 */
void UpdateTxPlanning (mcl_cb	*mclcb,
		       adu_t	*adu_start,
		       adu_t	*adu_end)
{
	static int	initialized = 0;

	ASSERT(adu_start && adu_end);
	if (!initialized) {
#ifdef LCT_SCHED1
		sched_tab[MCL_SCHED_LCT1] = LCT1_UpdateTxPlanning;
#endif
#ifdef LCT_SCHED2
		sched_tab[MCL_SCHED_LCT2] = LCT2_UpdateTxPlanning;
#endif
#ifdef LCT_SCHED3
		sched_tab[MCL_SCHED_LCT3] = LCT3_UpdateTxPlanning;
#endif
		initialized = 1;
	}
	ASSERT(sched_tab[mclcb->scheduler]);
	sched_tab[mclcb->scheduler](mclcb, adu_start, adu_end);
}


/*
 * Create a random permutation of (1..n)
 * (see: http://www.cs.berkeley.edu/~jfc/cs174/lecs/lec2.html)
 * Returns a table containing the random permutation.
 * WARNING: it is the caller's responsibility to free
 *	this array.
 */
#define mcl_random_permut(a,n)	mcl_pseudorandom_permut((a),(n),1)

/*
 * create a partially random permutation of (1..n)
 */
static void
mcl_pseudorandom_permut (int	*a,	/* input/output array */
			 int	n,	/* number of elements in array */
			 int	l)	/* permut one elt out of l */
{
	int	i, j, temp;

	ASSERT(l <= n);
	for (i = 0; i < n; i++)
		a[i] = i;
	for (i = 0; i < n; i+=l) {	/* skip some indexes */
		j = random() % n;
		temp = a[i];
		a[i] = a[j];
		a[j] = temp;
	}
}


/****** LCT scheduling version 1 **********************************************/
#if defined(LCT_SCHED1)		/* { */


/*
 * struct used to identify ADUs given a DU index
 */
typedef struct {
	adu_t	*adu;			/* for this ADU */
	int	first_idx;		/* DUs start at this (flat) index... */
	int	last_idx;		/* ... up to this one */
	int	du_nb;			/* total nb of data DUs in all blocks */
	/* fields required by LCT_SCHED2 */
	int	nb_fec_layers;		/* nb of FEC layers in LCT_SCHED2 */
} idx_range_t;


/* binary search more efficient with sessions having very large number of ADU */
#define BIN_SEARCH

#ifdef BIN_SEARCH
/*
 * binary tree search version...
 * usefull in case of very large ADU sessions
 */
static int
mcl_flat_index_to_adu_bsearch (idx_range_t	*idx_range_tab,
				int		low,  /* low index, included */
				int		high, /* high index, included */
				int		index)
{
	int	mid;	/* mid index */
	ASSERT(low <= high);
	/*if (low == high) { return low; }*/
	mid = ((low + high) >> 1);
	if (index <= idx_range_tab[mid].last_idx) {
		if (index >= idx_range_tab[mid].first_idx)
			return mid;	/* found */
		return mcl_flat_index_to_adu_bsearch(idx_range_tab, low, mid,
						     index);
	} else {
		return mcl_flat_index_to_adu_bsearch(idx_range_tab, mid+1, high,
						     index);
	}
}


/*
 * retrieve the idx_range_t struct of the adu corresponding to the flat
 * index given.
 */
#ifdef DEBUG
static idx_range_t*
mcl_flat_index_to_adu (mcl_cb		*mclcb,
		       idx_range_t	*idx_range_tab,
		       int		nb_entries,
		       int		index)
{
	int	idx;
	ASSERT(index >= 0 && index <= idx_range_tab[nb_entries-1].last_idx);
	idx = mcl_flat_index_to_adu_bsearch (idx_range_tab, 0,
					     nb_entries-1, index);
	TRACELVL(5, (mcl_stdout,
		"   mcl_flat_index_to_adu: index=%d, adu=x%x\n",
		index, (int)idx_range_tab[idx].adu))
	return &(idx_range_tab[idx]);
}
#else  /* DEBUG */
#define mcl_flat_index_to_adu(mclcb,idx_range_tab,nb_entries,index)	\
	&(idx_range_tab[mcl_flat_index_to_adu_bsearch(idx_range_tab, 0,	\
						      (nb_entries)-1, (index))])
#endif /* DEBUG */


#else  /* BIN_SEARCH */


/*
 * retrieve the idx_range_t struct of the adu corresponding to the flat
 * index given.
 */
static idx_range_t*
mcl_flat_index_to_adu (mcl_cb		*mclcb,
		       idx_range_t	*idx_range,
		       int		nb_entries,
		       int		index)
{
	/*TRACELVL(5, (mcl_stdout, "-> mcl_flat_index_to_adu: index=%d\n", index))*/
	ASSERT(index >= 0);
	while (1) {
		ASSERT(idx_range->adu != NULL);
		if (index <= idx_range->last_idx &&	/* more efficient */
		    index >= idx_range->first_idx) {	/* in this order! */
			/* found */
			TRACELVL(5, (mcl_stdout, "   mcl_flat_index_to_adu: idx=%d, adu=x%x\n",
				index, (int)idx_range->adu));
			return idx_range;
		}
		idx_range++;
	}
	ASSERT(0);	/* unreachable */
	return NULL;
}


#endif /* BIN_SEARCH */


static int	LCT1_mcl_get_tot_nb_of_du	(mcl_cb *mclcb, adu_t *adu);
static du_t*	LCT1_mcl_flat_index_to_du	(mcl_cb *mclcb, adu_t *adu, int index);


/*
 * Send everything on each layer.
 * Currently uses a straightforward algorithm: all the DUs are sent
 * on all the layers using a random permutation
 */
void
LCT1_UpdateTxPlanning (mcl_cb	*mclcb,
		  adu_t		*adu_start,	/* first ADU (included) */
		  adu_t		*adu_end)	/* last ADU (included) */
{
	int	i;		/* */
	int	j;		/* flat index in random permutation array */
	int	lay;		/* layer nb */
	txlay_t	*tl;
	int	*permut;	/* random permutation array */
	int	adu_nb;		/* number of ADUs */
	adu_t	*adu;
	int	du_nb;		/* number of DUs */
	int	tot_du_nb;	/* total DU nb (incl. FEC) for all blks, all ADUs */
	du_t	*du;
	idx_range_t *idx_range;	/* table */

	TRACELVL(5, (mcl_stdout, "-> LCT1_UpdateTxPlanning:\n"))
	if (adu_start == adu_end) {
		/* the easy traditional case: only one adu */
		/*
		 * tot_du_nb includes both plain DUs and FEC DUs of all blocks
		 */
		TRACELVL(5, (mcl_stdout, "   LCT1_UpdateTxPlanning: LCT org1/sequential object order\n"))
		adu = adu_start;
		tot_du_nb = LCT1_mcl_get_tot_nb_of_du(mclcb, adu);
		if (!(permut = (int*)malloc(tot_du_nb * sizeof(int)))) {
			goto no_memory;
		}
		/*
		 * for each layer, create a random permutation of all the DUs
		 * and register them
		 */
		for (lay = 0, tl = mclcb->txlay_tab; lay < mclcb->max_nb_layers;
		     lay++, tl++) {
			mcl_random_permut(permut, tot_du_nb);
			ASSERT(permut);
			for (j = 0; j < tot_du_nb; j++) {
				du = LCT1_mcl_flat_index_to_du(mclcb, adu, permut[j]);
				ASSERT(du);
#ifdef VIRTUAL_TX_MEM
				du->vtm_info.du_in_seq_in_txtab = 1;
#endif
				if (du->block->adu->priority == ADU_NORMAL_PRIO) {
					tl->tx_tab->register_du(mclcb, du,
								mclcb->nb_tx);
				} else {
					ASSERT(du->block->adu->priority == ADU_HIGH_PRIO);
					tl->tx_tab_high->register_du(mclcb, du,
								mclcb->nb_tx);
				}
							  
			}
		}
	} else if (mclcb->adu_scheduling == MCL_SCHED_RANDOM_OBJ_ORDER) {
		/* a bit more complex: list of adus */
		/*
		 * send ADUs in a different random order on each layer
		 */
		int		*permut2;	/* random DU permutation array*/
		idx_range_t	*ir;		/* table */
		int		max_du_nb;	/* max number of DUs in ADUs */

		TRACELVL(5, (mcl_stdout,
		"   LCT1_UpdateTxPlanning: LCT org1/random obj order\n"))
		adu_nb = adu_end->seq - adu_start->seq + 1;
		if (!(permut = (int*)malloc(adu_nb * sizeof(int))) ||
		    !(idx_range = (idx_range_t *)malloc(adu_nb * sizeof(idx_range_t)))) {
			goto no_memory;
		}
		max_du_nb = -1;
		for (i = 0, adu = adu_start, ir = idx_range;
		     i < adu_nb;
		     i++, adu = adu->next, ir++) {
			ir->du_nb = LCT1_mcl_get_tot_nb_of_du(mclcb, adu);
			if (ir->du_nb > max_du_nb)
				max_du_nb = ir->du_nb;
			ir->adu = adu;
			ir->first_idx = 0; /* register adu id only */
			ir->last_idx = 0;  /* register adu id only */
		}
		/* allocate this tab only once */
		if (!(permut2 = (int*)malloc(max_du_nb * sizeof(int)))) {
			goto no_memory;
		}
		/*
		 * for each layer, create a random permutation of all the ADUs
		 * and then a random permutation of all the DUs for each ADU.
		 */
		for (lay = 0, tl = mclcb->txlay_tab; lay < mclcb->max_nb_layers;
		     lay++, tl++) {
			mcl_random_permut(permut, adu_nb);
			for (i = 0; i < adu_nb; i++) {
				adu = idx_range[permut[i]].adu;
				du_nb = idx_range[permut[i]].du_nb;
				mcl_random_permut(permut2, du_nb);
				for (j = 0; j < du_nb; j++) {
					du = LCT1_mcl_flat_index_to_du(mclcb,
							adu, permut2[j]);
					ASSERT(du);
#ifdef VIRTUAL_TX_MEM
					du->vtm_info.du_in_seq_in_txtab = 1;
#endif
					if (du->block->adu->priority == ADU_NORMAL_PRIO) {
						tl->tx_tab->register_du(mclcb,
							du, mclcb->nb_tx);
					} else {
						ASSERT(du->block->adu->priority == ADU_HIGH_PRIO);
						tl->tx_tab_high->register_du(mclcb,
							du, mclcb->nb_tx);
					}
				}
			}
		}
	} else {
		/* a bit more complex: list of adus */
		/*
		 * mix everything: all the DUs of all the ADUs are sent
		 * on all the layers using a random permutation
		 */
		/*
		 * how many DUs per ADU; remember it for future index->du access
		 */
		idx_range_t	*idx_entry;
		int		idx;		/* index */

		TRACELVL(5, (mcl_stdout, "   LCT1_UpdateTxPlanning: LCT org1/mixed order\n"))
		adu_nb = adu_end->seq - adu_start->seq + 1;
		if (!(idx_range = (idx_range_t *)malloc(adu_nb * sizeof(idx_range_t)))) {
			goto no_memory;
		}
		tot_du_nb = 0;
		for (i = 0, adu = adu_start; i < adu_nb; i++, adu = adu->next) {
			du_nb = LCT1_mcl_get_tot_nb_of_du(mclcb, adu);
			idx_range[i].adu = adu;
			idx_range[i].first_idx = tot_du_nb;
			tot_du_nb += du_nb;
			idx_range[i].last_idx = tot_du_nb - 1;
		}
		if (!(permut = (int*)malloc(tot_du_nb * sizeof(int)))) {
			goto no_memory;
		}
		/*
		 * for each layer, create a random permutation of all the DUs
		 * and register them
		 */
		for (lay = 0, tl = mclcb->txlay_tab; lay < mclcb->max_nb_layers;
		     lay++, tl++) {
			/*mcl_random_permut(permut, tot_du_nb);*/
			if (mclcb->adu_scheduling == MCL_SCHED_MIXED_ORDER) {
				mcl_random_permut(permut, tot_du_nb);
			} else if (mclcb->adu_scheduling == MCL_SCHED_PARTIALLY_MIXED_ORDER) {
				/*
				 * if you want you can change the spacing parameter
				 * to modify the probability for a DU not to be
				 * permutted. The higher the value, the less random!
				 */
				mcl_pseudorandom_permut(permut, tot_du_nb, 2);
			} else {
				PRINT_ERR((mcl_stderr, "LCT1_UpdateTxPlanning: ERROR, ADU scheduler %d not possible in LCT1\n", mclcb->adu_scheduling))
				mcl_exit(-1);
			}
			ASSERT(permut);
			for (j = 0; j < tot_du_nb; j++) {
				idx_entry = mcl_flat_index_to_adu(mclcb,
						idx_range, adu_nb, permut[j]);
				adu = idx_entry->adu;
				idx = permut[j] - idx_entry->first_idx;
				ASSERT(idx >= 0);
				ASSERT(adu);
				du = LCT1_mcl_flat_index_to_du(mclcb, adu, idx);
				ASSERT(du);
#ifdef VIRTUAL_TX_MEM
				du->vtm_info.du_in_seq_in_txtab = 1;
#endif
				if (du->block->adu->priority == ADU_NORMAL_PRIO) {
					tl->tx_tab->register_du(mclcb, du,
								mclcb->nb_tx);
				} else {
					ASSERT(du->block->adu->priority == ADU_HIGH_PRIO);
					tl->tx_tab_high->register_du(mclcb, du,
								mclcb->nb_tx);
				}
			}
		}
		free(idx_range);
	}
	free(permut);
	TRACELVL(5, (mcl_stdout, "<- LCT1_UpdateTxPlanning:\n"))
	return;

no_memory:
	PRINT_ERR((mcl_stderr, "LCT1_UpdateTxPlanning: ERROR, no memory"))
	mcl_exit(-1);
}


static int
LCT1_mcl_get_tot_nb_of_du (mcl_cb	*mclcb,
		      adu_t	*adu)
{
	int	i;
	block_t	*blk;
	int	nb = 0;

	/*TRACELVL(5, (mcl_stdout, "-> LCT1_mcl_get_tot_nb_of_du:\n"))*/
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head; i > 0; i--, blk++) {
		nb += blk->k;
#ifdef FEC
		nb += blk->fec_du_nb_in_list;
#endif
	}
	TRACELVL(5, (mcl_stdout, "   LCT1_mcl_get_tot_nb_of_du: %d du\n", nb))
	return nb;
}


/*
 * retrieve the du corresponding to the (flat) index given.
 */
static du_t*
LCT1_mcl_flat_index_to_du (mcl_cb	*mclcb,
			   adu_t	*adu,
			   int		index)
{
	int	i;
	block_t	*blk;

	/*TRACELVL(5, (mcl_stdout, "-> LCT1_mcl_flat_index_to_du: index=%d\n", index))*/
	ASSERT(index >= 0);
	ASSERT(index < LCT1_mcl_get_tot_nb_of_du(mclcb, adu));
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head;
	     i > 0; i--, blk++) {
		if (index < (int)blk->k) {
			TRACELVL(5, (mcl_stdout,
			"   LCT1_mcl_flat_index_to_du: idx=%d, du=x%x\n",
				index, (int)&(blk->du_head[index])))
			return &(blk->du_head[index]);
		}
		index -= blk->k;
#ifdef FEC
		if (index < (int)blk->fec_du_nb_in_list) {
			TRACELVL(5, (mcl_stdout,
			"   LCT1_mcl_flat_index_to_du: fec idx=%d, du=x%x\n",
				index, (int)&(blk->du_head[index])))
			return &(blk->fec_du_head[index]);
		}
		index -= blk->fec_du_nb_in_list;
#endif
	}
	ASSERT(0);	/* unreachable */
	return NULL;
}


#endif /* } */


/****** LCT scheduling for ANTICIPATED_TX_FOR_PUSH ****************************/
#if defined(ANTICIPATED_TX_FOR_PUSH)	/* { */


static void AnticipTx_register_du_on_right_layer (mcl_cb *mclcb, du_t *du);


/*
 * Create a linear and vertical scheduling of DUs, i.e. accross all layers.
 * Always called with a single adu.
 * No FEC transmission, only data.
 * No randomization here, linear scheduling.
 */
void
AnticipTx_UpdateTxPlanning (mcl_cb	*mclcb,
			    adu_t	*adu)
{
	block_t	*blk;
	int	b_nb;		/* nb of blocks */
	int	du_nb;		/* nb of dus */
	du_t	*du;

	TRACELVL(5, (mcl_stdout, "-> AnticipTx_UpdateTxPlanning:\n"))
	/*
	 * register all the data DUs of this ADU
	 */
	for (b_nb = adu->blocking_struct.block_nb, blk = adu->block_head;
	     b_nb > 0; b_nb--, blk++) {
		for (du_nb = blk->k, du = blk->du_head; du_nb > 0;
		     du_nb--, du++) {
			AnticipTx_register_du_on_right_layer(mclcb, du);
		}
	}
	TRACELVL(5, (mcl_stdout, "<- AnticipTx_UpdateTxPlanning:\n"))
}


static void
AnticipTx_register_du_on_right_layer (mcl_cb	*mclcb,
				      du_t	*du)
{
	static int	layer = 0;		/* next tx is on that layer */
	static int	rem_2_tx_on_layer = 1;	/* can send that nb of du */

	TRACELVL(5, (mcl_stdout,
		"   AnticipTx_register_du_on_right_layer: lay=%d, rem2tx=%d\n",
		layer, rem_2_tx_on_layer))
	if (rem_2_tx_on_layer <= 0) {
		/* move to next layer */
		/* only consider first 5 or max_nb_layers layers */
		if (++layer >= mclcb->max_nb_layers || layer >= 5)
			layer = 0;
		/*
		 * consider the upper integer, to avoid rounding to 0
		 * with very low bit rates... This is not fully accurate,
		 * but we don't care in this particular case.
		 */
		rem_2_tx_on_layer = (INT32)ceil(mclcb->txlay_tab[layer].du_per_tick);
	}
	/* schedule for a SINGLE tx */
#ifdef VIRTUAL_TX_MEM
	du->vtm_info.du_in_seq_in_txtab = 1;
#endif
	if (du->block->adu->priority == ADU_NORMAL_PRIO) {
		mclcb->txlay_tab[layer].tx_tab->register_du(mclcb, du, 1);
	} else {
		ASSERT(du->block->adu->priority == ADU_HIGH_PRIO);
		mclcb->txlay_tab[layer].tx_tab_high->register_du(mclcb, du, 1);
	}
	rem_2_tx_on_layer--;
	/*TRACELVL(5, (mcl_stdout, "<- AnticipTx_register_du_on_right_layer: layer=%d\n", layer))*/
}


#endif /* ANTICIPATED_TX_FOR_PUSH } */

