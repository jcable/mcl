/* $Id: mcl_tx_window.h,v 1.3 2004/02/18 07:56:18 roca Exp $ */
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

#ifndef MCL_TX_WINDOW_H
#define MCL_TX_WINDOW_H



/**
 * This class provides all the required services to manage the transmission
 * window at the sender.
 * It enables all the data DUs of an ADU plus its FEC DUs (if any) to be
 * registered for transmission.
 */
class mcl_tx_window {
 
public:
	/****** Public Members ************************************************/
	/**
	 * default constructor
	 */
	mcl_tx_window ();

	/**
	 * default destructor
	 */
	~mcl_tx_window ();

	/**
	 * Is it possible to register a new ADU?
	 * Performs flow control between the upper application and MCL.
	 * For simplicity, does not take into account the ADU data size,
	 * it just checks if there is any room left.
	 * This is not required for DU registration, since they are
	 * generated internally by MCL, not by the application.
	 * @return	boolean
	 */
	bool		can_register_new_adu (mcl_cb *const mclcb);

	/**
	 * Register a new adu (i.e. all of its data DUs and FEC DUs if any)
	 * for transmission in the sending window.
	 * The DUs are inserted in a NORMAL PRIORITY list.
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	register_new_adu (mcl_cb	*const mclcb,
						  mcl_adu	*const adu);

	/**
	 * Register a new FEC or data DU for transmission in the sending
	 * window.
	 * The DUs are inserted in a HIGH PRIORITY list.
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	register_new_du (mcl_cb	*const mclcb,
						 mcl_du	*const du);

	/**
	 * * Reschedule transmissions (data and/or FEC DUs) for a block.
	 * Reschedule the transmission of all available FEC, and if not
	 * sufficient, retransmit data.
	 * @param mclcb
	 * @param blk		block that must be retransmitted entirely
	 * @param data_required	nb of data DUs to transmit (>= 0)
	 * @param fec_required	nb of FEC DUs to transmit (>= 0)
	 */
	mcl_error_status	reschedule_tx_for_block (mcl_cb	*const mclcb,
						mcl_block	*blk,
						INT32		data_required,
						INT32		fec_required);

	/**
	 * Remove a block from the transmission window.
	 * Called usually when the block has been sent reliably (e.g.
	 * at the end of a FLUSH_DONE process).
	 * @param mclcb
	 * @param blk	block to remove
	 * @param fast_mode	in fast mode it is assumed there is no pending
	 * 			DU transmission for this block
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_block (mcl_cb	*const mclcb,
					      mcl_block	*const blk,
					      bool	fast_mode);

	/**
	 * Return the number of available data and FEC DUs for transmissions
	 * in either the normaL or high priority DU list.
	 * The returned value may be smaller than the total number of du
	 * registered, because some DUs may have already been sent.
	 */
	INT32		get_nb_of_available_du_to_tx ();

	/**
	 * Return the number of data or FEC du registered in the normal
	 * priority list.
	 * This value is most of the time strictly larger than that
	 * returned by get_nb_of_available_du_to_tx().
	 */
	INT32		get_nb_of_registered_du ();

	/**
	 * Return the number of data or FEC du registered in the high
	 * priority list.
	 * This value is most of the time small since they usually concern
	 * retransmitted DUs and additional FEC DUs.
	 */
	INT32		get_nb_of_registered_hp_du ();

	/**
	 * Returns the next DU to send, in either of the two lists.
	 * This DU is automatically removed from the associated list.
	 */
	mcl_du		*get_next_du_to_tx (mcl_cb *const mclcb);

	/****** Public Attributes *********************************************/
  
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
  
private:
	/****** Private Members ***********************************************/
	/**
	 * Insert a DU in the normal priority list.
	 * @param du	data or FEC DU to insert
	 */
	void	insert_in_list (mcl_cb *const mclcb, mcl_du *const du);

	/**
	 * Insert a DU in the high priority list.
	 * @param du	data or FEC DU to insert
	 */
	void	insert_in_hp_list (mcl_cb *const mclcb, mcl_du *const du);

	/**
	 * Remove a DU from the normal priority list, if found.
	 * @param du	data or FEC DU to remove
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_from_list (mcl_cb	*const mclcb,
						  mcl_du	*const du);

	/**
	 * Remove a DU from the normal priority list, if found.
	 * @param du	data or FEC DU to remove
	 * @return	completion status (MCL_OK or MCL_ERROR).
	 */

	mcl_error_status	remove_from_hp_list (mcl_cb	*const mclcb,
						     mcl_du	*const du);

	/****** Private Attributes ********************************************/

	/**
	 * Sending application / MCL flow control system:
	 *
	 * There is a tx window, consisting in a doubly linked list of blocks.
	 * Some of these blocks:
	 * 	- have been sent and ack'ed... They'll probably soon be free'd.
	 * 	- have been fully sent but not yet fully ack'ed... The sender
	 * 		is still waiting for ACKs/NAKs and is ready for retx.
	 * 	- have been partially sent
	 * 		Next DU transmissions will probably take place for
	 * 		this block.
	 * 	- have not yet been sent
	 *
	 * The following variables are used:
	 * blist_head		head of list of blocks in tx window
	 * blist_tail		its tail
	 * blist_snd_una	oldest unacknowledged block sent
	 * blist_snd_max	transmissions have occured up to this block
	 * blist_size		theoretical window size (in blocks)
	 * blist_eff_size	effective size (in blocks)
	 *
	 * ========>  sent and ack'ed
	 * ============ sent blocks =========>
	 *           <======= window size ============>
	 *           <======= effective size ================>
	 * ---------[+-----------------------+]--------------+]----> [block] #
	 * ^         ^                        ^              ^
	 * |         |                        |              |
	 * head   snd_una                  snd_max         tail
	 *
	 * There can be more blocks in list than in theory 
	 * (i.e. blist_eff_size > blist_size) because ADUs are submitted
	 * as a whole, and all the blocks are inserted in the list,
	 * even if they exceed the expected window size.
	 * The advantage is that an ADU will always be accepted as a whole,
	 * never partially.
	 * A major drawback is that very large ADUs can easily fool the
	 * system. So the ADU size should be kept reasonable (e.g. the
	 * application should segment huge files if necessary).
	 */

	/** Block list (blist). */
	mcl_block	*blist_head;
	mcl_block	*blist_tail;
	mcl_block	*blist_snd_una;
	mcl_block	*blist_snd_max;
	INT32		blist_size;		// in blocks
	INT32		blist_eff_size;		// in blocks
	static const INT32 DFLT_BLIST_SIZE = 100;	// default, in blocks

	/**
	 * And here is the corresponding DU lists.
	 * There is one for normal DUs (usually data DUs, sometimes also FEC
	 * DUs if proactive FEC is used), and another one for high priority
	 * DUs (e.g. in case of retransmissions, or if additional FEC DUs are
	 * calculated for loss recovery).
	 * The following policy applies:
	 * 	- serve the FEC list in priority. These FEC DUs are created
	 * 		in case for loss recovery and should be sent first
	 * 		whenever some exist.
	 * 	- never send a FEC DU before the first data DU of an ADU.
	 * 		in ever it would took place, then move the FEC DU
	 * 		in the du_list at the corresponding place.
	 * 	- then serve the normal DU list.
	 * 	- once transmitted, DUs are automatically removed from the
	 * 		list. It does not mean they are free'd though. In
	 * 		order to schedule a retransmission, the DU should
	 * 		be registered once again.
	 */
	/** Normal DU list (du_list). */
	mcl_du		*du_list_head;
	/** transmission already took place up to and including this DU. */
	mcl_du		*du_list_snd_max;
	/** counter */
	INT32		nb_of_du_in_list;

	/** High Priority DU list (hp_list). */
	mcl_du		*hp_du_list_head;
	/** transmission already took place up to and including this DU. */
	mcl_du		*hp_du_list_snd_max;
	/** counter */
	INT32		nb_of_du_in_hp_list;

	/**
	 * Serve MAX_SEQ_TX_FROM_HP pkts from the HP list, and then one
	 * from the normal list.
	 */
	static const INT32 MAX_SEQ_TX_FROM_HP = 2;
	INT32		seq_tx_in_hp_count;	/** counter */

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline INT32
mcl_tx_window::get_nb_of_available_du_to_tx ()
{
	return (this->nb_of_du_in_list + this->nb_of_du_in_hp_list);
}

#endif // !MCL_TX_WINDOW_H
