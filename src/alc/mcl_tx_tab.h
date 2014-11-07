/* $Id: mcl_tx_tab.h,v 1.8 2005/04/07 15:51:39 moi Exp $ */
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

#ifndef MCL_TX_TAB_H  /* { */
#define MCL_TX_TAB_H


/*
 * Table of planned transmissions on a layer.
 * Each struct contains at most NB_TXTAB_ENTRIES each, and more struct follow
 * if required. All DUs of a txtab are sent the same number of times.
 */
#define NB_TXTAB_ENTRIES	1000
typedef struct txtab {
	struct txtab	*prev, *next;
	UINT32		wr_index;	/* index of next free DU in txtab */
	UINT32		tx_index;	/* index of next DU to send in txtab */
	INT32		rem_tx;		/* remaining number of tx for each du */
	du_t		*du_tab[NB_TXTAB_ENTRIES]; /* table of DUs to send */
} txtab_t;


/**
 * Class defining the transmission plannification service at a sender.
 * There is an instance of this class for normal priority ADUs, and another
 * one for high priority ADUs.
 */
class mcl_tx_tab {

public:
	/****** Public Members ************************************************/
	/** Default constructor. */
	mcl_tx_tab ();

	/** Default destructor. */
	~mcl_tx_tab ();

	/**
	 * Register a du for nb_tx transmissions on that layer.
	 * @param mclcb
	 * @param du		DU to register
	 * @param nb_tx		Desired number of transmissions for this DU.
	 * 			This parameter is registered in the entry
	 * 			associated with this DU and is decremented
	 * 			after each transmission (unless we are in
	 * 			ON-DEMAND mode).
	 */
	void	register_du (class mcl_cb *const mclcb, du_t *du, INT32 nb_tx);

	/**
	 * Returns the next DU to tx for this layer or NULL.
	 * @param mclcb
	 * @return
 	 */
	du_t*	get_next_du (class mcl_cb *const mclcb);


	/**
	 * adds new transmission cycles.
	 * @return	 0 if ok, < 0 if an error
	 */
	INT32 add_tx_cycles (mcl_cb	*const mclcb, INT32 nb_cycles);

	/**
	 * Reset the txtab for a new transmission cycle in on demand
	 * delivery mode.
	 * @return		returns 0 if ok, < 0 if an error
	 */
	INT32	new_tx_cycle (class mcl_cb *const mclcb);

	/**
	 * Look in the txtab and mark the ADU of all the DUs waiting to be sent
	 * as "in_txtab".
	 * @param mclcb
	 */
	void	mark_adus_in_txtab (class mcl_cb *const mclcb);

	/**
	 * Free all the txtab structs for this mclcb.
	 */
	void	free_all_txtab ();

	/**
	 *  remove all adus that are in txtabs 
	 * and adds the removed adus to the adu_list
	 * @param mclcb
	 * @param adu_list
	 */
	void 	remove_all_adu_from_txtab (mcl_cb *const mclcb, adu_t ** adu_list);

	/****** Public Attributes *********************************************/

	int		tot_rem;	/* number of DUs remaining to be sent */
  

private:
	/****** Private Members ***********************************************/

	/**
	 * Create and init a new txtab_t struct.
	 * @param mclcb
	 */
	txtab_t *	create_txtab (mcl_cb *const mclcb);

	/**
	 * Insert the txtab list (one or more txtab) at the end of the list.
	 * Returns 1 if list was empty, 0 otherwise.
	 * @param mclcb
	 */
	INT32 		insert_txtab (mcl_cb *const mclcb, txtab_t *tt);

	/**
	 * Remove this txtab (there's only one) from the list.
	 */
	void		remove_txtab (txtab_t *tt);

	/**
	 * Returns the txtab found (if any), NULL if nothing is available.
	 * @param mclcb
	 */
	txtab_t *	get_first_ready_txtab (mcl_cb *const mclcb);

	/**
	 * Get last element in txtab.
	 */
	txtab_t	*	last_txtab ();


	/****** Private Attributes ********************************************/

	txtab_t		*txtab_head;	/* tx plannification table */
	txtab_t		*cached_txtab;	/* next DU to transmit belongs to */
					/* this txtab */
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


inline txtab_t	*
mcl_tx_tab::last_txtab ()
{
	return ((this->txtab_head) ? ((this->txtab_head)->prev) : NULL);
}


#endif /* } MCL_TX_TAB_H */
