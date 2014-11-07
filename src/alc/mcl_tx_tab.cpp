/* $Id: mcl_tx_tab.cpp,v 1.11 2005/04/07 15:51:39 moi Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - All rights reserved
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



/****** Public Functions ******************************************************/

mcl_tx_tab::mcl_tx_tab ()
{
	this->txtab_head = NULL;
	this->cached_txtab = NULL;
	this->tot_rem = 0;
}


mcl_tx_tab::~mcl_tx_tab ()
{
	this->free_all_txtab();
}


/*
 * Register a du for nb_tx transmissions on that layer.
 */
void
mcl_tx_tab::register_du (class mcl_cb	*const mclcb,
			 du_t		*du,
			 INT32		nb_tx)
{
	txtab_t	*tt;

#ifdef FEC
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::register_du: du=x%x (aseq=%d bseq=%d dseq=%d/%s)\n",
		(int)du, du->block->adu->seq, du->block->seq,
		du->seq, (du->is_fec ? "fec" : "data")))
#else
	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::register_du: du=x%x (aseq=%d bseq=%d dseq=%d)\n",
		(int)du, du->block->adu->seq, du->block->seq, du->seq))
#endif
	ASSERT(mclcb && du);
	if (nb_tx <= 0) {
		PRINT_ERR((mcl_stderr, "mcl_tx_tab::register_du: ERROR, nb_tx(%d)<=0", nb_tx))
		mcl_exit(-1);
	}
	tt = this->last_txtab();
	if (tt == NULL || tt->wr_index >= NB_TXTAB_ENTRIES || tt->rem_tx != nb_tx) {
		/* need a new txtab struct */
		tt = this->create_txtab(mclcb);
		this->insert_txtab(mclcb, tt);
	}
	tt->du_tab[tt->wr_index++] = du;
	tt->rem_tx = nb_tx;
	this->tot_rem++;

	//printf("register du, wr_index %i, tx_index %i\n",tt->wr_index,tt->tx_index);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::register_du:\n"))
}


/*
 * Returns the next DU to tx for this layer or NULL.
 */
du_t *
mcl_tx_tab::get_next_du (mcl_cb		*const mclcb)
{
	txtab_t		*tt;
	du_t		*du;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::get_next_du:\n"))
	ASSERT(mclcb);
	if (this->tot_rem <= 0) {
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::get_next_du: NULL (empty)\n"))
		return NULL;
	}
	/*TRACELVL(5, (mcl_stdout, "   mcl_get_next_du: before: tl->cached_txtab=x%x\n", (int)tl->cached_txtab)) */
	if ((tt = this->cached_txtab) == NULL) {	/* cache fault */
		if ((tt = this->get_first_ready_txtab(mclcb)) == NULL) {
			PRINT_ERR((mcl_stderr, "mcl_tx_tab::get_next_du: ERROR, null tt"))
			mcl_exit(-1);
		}
		this->cached_txtab = tt;		/* remember it */
	}
	if (tt->tx_index >= tt->wr_index) {	/* exhausted: move to next tt */
		if ((tt = this->get_first_ready_txtab(mclcb)) == NULL) {
			PRINT_ERR((mcl_stderr, "mcl_tx_tab::get_next_du: ERROR, null tt"))
			mcl_exit(-1);
		}
		this->cached_txtab = tt;		/* remember it */
	}
	/*TRACELVL(5, (mcl_stdout, "   mcl_get_next_du: after: tl->cached_txtab=x%x\n", (int)tl->cached_txtab)) */
	ASSERT(tt->tx_index < tt->wr_index);
	ASSERT(tt->wr_index <= NB_TXTAB_ENTRIES &&
	       tt->tx_index < NB_TXTAB_ENTRIES);
	du = tt->du_tab[tt->tx_index];
	tt->tx_index++;
	this->tot_rem--;
#ifdef FEC
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::get_next_du: du=x%x (aseq=%d bseq=%d dseq=%d/%s)\n",
		(int)du, du->block->adu->seq, du->block->seq,
		du->seq, (du->is_fec ? "fec" : "data")))
#else
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::get_next_du: du=x%x (aseq=%d bseq=%d dseq=%d)\n",
		(int)du, du->block->adu->seq, du->block->seq, du->seq))
#endif

	return du;
}


/*
 * reset the txtab for a new transmission cycle if needed.
 * returns 0 if ok, < 0 if an error
 */
INT32
mcl_tx_tab::new_tx_cycle (mcl_cb	*const mclcb)
{
	txtab_t	*tt;
	txtab_t	*tmp_tt;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::new_tx_cycle:\n"))
	ASSERT(mclcb);
	ASSERT(this->tot_rem == 0);
	if (!(tt = this->txtab_head)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::new_tx_cycle: no txtab\n"))
		return -1;
	}
	do {
		if (mclcb->delivery_mode != DEL_MODE_ON_DEMAND) {
			/* in on-demand, tx for ever, otherwise decrement */
			tt->rem_tx--;
		}
		if (tt->rem_tx <= 0) {
			/* cannot be used for tx anymore, free it */
			tmp_tt = tt;
			tt = tt->next;
			this->remove_txtab(tmp_tt);
			free(tmp_tt);
		} else {
			this->tot_rem += tt->wr_index;
			tt->tx_index = 0;
			tt = tt->next;
		}
	} while (this->txtab_head && tt != this->txtab_head);
	this->cached_txtab = NULL;	/* reset cache */
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::new_tx_cycle:\n"))
	return 0;
}


/*
 * adds new transmission cycles.
 * returns 0 if ok, < 0 if an error
 */
INT32
mcl_tx_tab::add_tx_cycles (mcl_cb	*const mclcb, INT32 nb_cycles)
{
	txtab_t	*tt;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::add_tx_cycles:\n"))
	ASSERT(mclcb);
	ASSERT(this->tot_rem == 0);
	if (!(tt = this->txtab_head)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::add_tx_cycles: no txtab\n"))
		return -1;
	}
	do {
		if (mclcb->delivery_mode != DEL_MODE_ON_DEMAND) {
			/* in on-demand, tx for ever, otherwise decrement */
			tt->rem_tx += nb_cycles;
		}
		tt = tt->next;
	} while (this->txtab_head && tt != this->txtab_head);
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::add_tx_cycles:\n"))
	return 0;
}


/*
 * look in the txtab and mark the ADU of all the DUs waiting to be sent
 * as "in_txtab"
 */
void
mcl_tx_tab::mark_adus_in_txtab (mcl_cb		*const mclcb)
{
	txtab_t		*tt;
	du_t		*du;
	INT32		i;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::mark_adus_in_txtab\n"))
	ASSERT(mclcb);
	tt = this->txtab_head;
	ASSERT(tt != NULL);
	do {
		if (tt->wr_index > 0 && tt->tx_index < tt->wr_index) {
			TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::mark_adus_in_txtab: mark DUs of tt=x%x\n", (int)tt))
			for (i = 0; i + tt->tx_index < tt->wr_index; i++) {
				du = tt->du_tab[tt->tx_index + i];
				/* mark this adu as "in_txtab" */
				du->block->adu->in_txtab = 1;
			}
		}
		tt = tt->next;
	} while (tt != this->txtab_head);
	/* we have cycled */

	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::mark_adus_in_txtab:\n"))
	return;
}


/*
 * free all the txtab structs for this mclcb
 */
void
mcl_tx_tab::free_all_txtab ()
{
	txtab_t	*tt;

	//TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::free_all_txtab:\n"))
	//ASSERT(mclcb->is_a_sender());
	while ((tt = this->last_txtab()) != NULL) {
		this->remove_txtab(tt);
		free(tt);
	}
	this->cached_txtab = NULL;	/* reset cache */
	//TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::free_all_txtab:\n"))
}


/*
 * remove all adus that are in txtabs and adds the removed adus to the adu_list
 */
void
mcl_tx_tab::remove_all_adu_from_txtab (mcl_cb	*const mclcb, adu_t ** adu_list)
{
	adu_t		*new_adu = NULL;
	INT32		j;
	txtab_t		*tt;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::remove_all_adu_from_txtab:\n"))
	ASSERT(mclcb->is_a_sender());
	
	while ((tt = this->last_txtab()) != NULL) 
	{
		for (j=0; j<NB_TXTAB_ENTRIES; j++) {
			if (tt->du_tab[j]!=NULL) {
				if(tt->du_tab[j]->block!=NULL) {
					if((new_adu = tt->du_tab[j]->block->adu )!=NULL) {
						if (mclcb->tx.get_first_adu(mclcb) != NULL)
						{
							mclcb->tx.remove_adu(mclcb, new_adu);
						}
						if ( mcl_find_adu (mclcb,new_adu->seq, new_adu->FDT_instanceid,*adu_list) == NULL)
						{
						 	mcl_insert_adu (mclcb, new_adu, &(*adu_list));
						}
					}
				}
			}
				
		}
		this->remove_txtab(tt);
		free(tt);
	}
	
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::remove_all_adu_from_txtab:\n"))
}

/****** Private Functions *****************************************************/


/*
 * Create and init a new txtab_t struct.
 */
txtab_t *
mcl_tx_tab::create_txtab (mcl_cb *const mclcb)
{
	txtab_t	*tt;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::create_txtab:\n"))
	if (!(tt = (txtab_t*)calloc(1, sizeof(txtab_t)))) {
		PRINT_ERR((mcl_stderr, "mcl_tx_tab::create_txtab: ERROR, no memory"))
		mcl_exit(-1);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::create_txtab: return x%x\n", (int)tt))
	return (tt);
}


/*
 * Insert the txtab list (one or more txtab) at the end of the list.
 * Returns 1 if list was empty, 0 otherwise.
 */
INT32
mcl_tx_tab::insert_txtab (mcl_cb	*const mclcb,
			  txtab_t	*tt)
{
	txtab_t	*ptt, *ntt;	 /* insert tt between prev_tt, next_tt */

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::insert_txtab:\n"))
	ASSERT(tt)
	ptt = this->last_txtab();	/* last txtab in list */
	if (ptt == NULL) {
		/*
		 * first txtab in list
		 */
		this->txtab_head = tt;
		if (!tt->next) {
			/* single txtab to add */
			ASSERT((!tt->prev))
			tt->next = tt->prev = tt;
		}
		TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::insert_txtab: return 1\n"))
		return 1;
	}
	ntt = ptt->next;	/* this is the following txtab in list */
	if (!tt->next) {
		/* single txtab to add */
		ASSERT((!tt->prev))
		ptt->next = tt;
		tt->prev = ptt;
		tt->next = ntt;
		ntt->prev = tt;
	} else {
		/* tt is a txtab list */
		ASSERT((tt->prev))
		ptt->next = tt;
		(tt->prev)->next = ntt;
		ntt->prev = (tt->prev);
		tt->prev = ptt;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::insert_txtab: return 0\n"))
	return 0;
}


/*
 * Remove this txtab (there's only one) from the list.
 */
void
mcl_tx_tab::remove_txtab (txtab_t	*tt)
{
	//TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::remove_txtab:\n"))
	ASSERT(tt);
#ifdef DEBUG
	{
	/* check that tt is in list (start from the end) */
	txtab_t	*ltt;
	for (ltt = this->last_txtab();
	     ltt != NULL && ltt != tt && ltt != this->txtab_head;
	     ltt = ltt->prev);
	ASSERT(ltt == tt);
	}
#endif /* DEBUG */
	/* found, remove it */
	ASSERT((tt->prev))
	ASSERT((tt->next))
	if (this->txtab_head == tt) {
		if (tt->next != tt) {
			ASSERT(tt->prev != tt);
			this->txtab_head = tt->next;
		} else {
			ASSERT(tt->prev == tt);
			this->txtab_head = NULL;
		}
	}
	tt->prev->next = tt->next;
	tt->next->prev = tt->prev;
	//TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::remove_txtab:\n"))
}


/*
 * Returns the txtab found (if any), NULL if nothing is available.
 */
txtab_t *
mcl_tx_tab::get_first_ready_txtab (mcl_cb	*const mclcb)
{
	txtab_t	*tt;

	TRACELVL(5, (mcl_stdout, "-> mcl_tx_tab::get_first_ready_txtab:\n"))
	tt = this->txtab_head;
	ASSERT(tt != NULL);
	do {
		if (tt->wr_index > 0 && tt->tx_index < tt->wr_index) {
			ASSERT(tt->wr_index <= NB_TXTAB_ENTRIES &&
			       tt->tx_index < NB_TXTAB_ENTRIES);
			TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::get_first_ready_txtab: found tt=x%x\n", (int)tt))
			return tt;
		}
		tt = tt->next;
	} while (tt != this->txtab_head);
	/* we have cycled */
	TRACELVL(5, (mcl_stdout, "<- mcl_tx_tab::get_first_ready_txtab: NULL (cycled)\n"))
	return NULL;
}

