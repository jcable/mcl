/* $Id: mcl_tx_ctrl.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_TX_CTRL_H
#define MCL_TX_CTRL_H

/**
 * Sender's control tasks.
 * This class is used for NORM control message transmission and reception
 * (if applicable) at a sender.
 * This class inherits the timer_handler class in order to have a timer
 * callback function.
 */
class mcl_tx_ctrl : public mcl_timer_handler {

public:
	/****** Public Members ************************************************/

	/**
	 * Default constructor.
	 */
	mcl_tx_ctrl();

	/**
	 * See if any control packet must be sent or not, and send it if
	 * necessary.
	 * This function is called just after the transmission of a DU
	 * (end_session false) or at the end of the session (end_session
	 * true).
	 * @param mclcb
	 * @param du	DU that has just been sent
	 * @param end_session	boolean true after the transmission of
	 * 		the last DU of the last block of a session.
	 * 		Used to trigger the tx of a FLUSH
	 */
	void	check_if_ctrl_pkt_must_be_sent (mcl_cb	*const mclcb,
						mcl_du	*du,
						bool	end_session);

	/**
	 * Send NORM_CMD(FLUSH) packet(s) and register a FLUSH_DONE
	 * timer event.
	 * @param mclcb
	 * @param blk	block for which we need to send a FLUSH
	 */
	void	send_flush_and_register_tevent (mcl_cb		*const mclcb,
						mcl_block	*blk);

	/**
	 * Send NORM_CMD(FLUSH) packet(s).
	 * @param mclcb
	 * @param blk	block for which we need to send a FLUSH
	 */
	void	send_flush_pkt (mcl_cb		*const mclcb,
				mcl_block	*blk);

	/**
	 * Send NORM_CMD(NO_NEW_OBJECT) packet(s).
	 */
	void	send_no_new_object_pkt (mcl_cb	*const mclcb);

	/**
	 * Send NORM_CMD(CLOSE) packet(s).
	 */
	void	send_close_pkt (mcl_cb	*const mclcb);

	/**
	 * Process an incoming NORM_NACK packet.
	 * @param mclcb
	 * @param pkt           pointer to packet
	 * @param saddr         source address for this packet
	 * @param chdr_infos    pointer to the common_infos struct
	 */
	mcl_error_status  process_nack_pkt (mcl_cb		*const mclcb,
					    class mcl_rx_pkt	*pkt,
					    mcl_addr		*saddr,
					    mcl_common_hdr_infos_t *chdr_infos);

	/**
	 * timer callback method.
	 * @param arg1          mclcb
	 * @param arg2          pointer to a tx_ctrl_timer_event_t struct
	 */
	virtual void timer_callback (INT32 arg1, INT32 arg2);

	/**
	 * See if there is one or more registered pending control
	 * operations (i.e. tevent).
	 * @param mclcb
	 * @return	true if there is one or more pending ctrl operation
	 */
	bool	is_there_pending_ctrl (mcl_cb	*const mclcb);


	/****** Public Attributes *********************************************/
 
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */

private:
	/****** Private Members ***********************************************/

	/**
	 * Send NORM_CMD(any flavor) packet(s).
	 * Sends MCL_TX_ROBUSTNESS_FACTOR copies of the packet.
	 * Used by send_no_new_object_pkt() and send_close_pkt() functions.
	 * @param mclcb
	 * @param flavor	flavor of the command packet
 	 * @param du		only required for NORM_CMD_FLUSH
	 */
	void	send_cmd_pkt (mcl_cb		*const mclcb,
			      norm_cmd_flavor	flavor,
			      mcl_du		*du);

	/****** Private Attributes ********************************************/

	/**
	 * Used by check_if_ctrl_pkt_must_be_sent() to identify a change
	 * of block and/or ADU being sent.
	 */
	mcl_block	*blk_of_last_du;

 	/**
	 * Create a list of pending timer_events.
	 * Required for instance to avoid duplicated timer_events.
	 * There is one list per tevent type.
	 */
	class mcl_tx_ctrl_tevent	*FLUSH_DONE_tevent_head;
	class mcl_tx_ctrl_tevent	*RETX_DONE_tevent_head;
};


/**
 * Types of timer events.
 * WARNING: must be in line with list of mcl_tx_ctrl_tevent::get_type_string()
 */
enum mcl_tx_ctrl_tevent_types {
	TX_CTRL_TEVENT_INVALID = 0,
	TX_CTRL_TEVENT_FLUSH_DONE,
	TX_CTRL_TEVENT_RETX_DONE
};


/**
 * This base class describes timer events.
 * This is the second arg of timer_callback() method.
 */
class mcl_tx_ctrl_tevent {

public:
	/****** Public Members ************************************************/

	/**
	 * No default constructor.
	 */

	/**
	 * Default destructor.
	 */
	virtual	~mcl_tx_ctrl_tevent ();

#ifdef DEBUG
	/** Return the tevent type as a static string. */
	char	*get_type_string () const;
#endif

	/**
	 * Timer-specific processing function
	 */

        /**
	 * Insert a tevent in an unordered list.
	 * Called with the appropriate list head parameter.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 */
	mcl_error_status  insert_in_list (mcl_cb		*const mclcb,
					  mcl_tx_ctrl_tevent	**head);

	/**
	 * Remove a tevent from an unordered list.
	 * Called with the appropriate list head parameter.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 */
	mcl_error_status  remove_from_list (mcl_cb		*const mclcb,
					    mcl_tx_ctrl_tevent	**head);


	/****** Public Attributes *********************************************/

	mcl_tx_ctrl_tevent_types	type;


protected:
	/****** Protected Members *********************************************/
	/****** Protected Attributes ******************************************/
	mcl_tx_ctrl_tevent	*prev, *next;


private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/

};


/**
 * This class describes the FLUSH_DONE timer event.
 * It is derived from the mcl_tx_ctrl_tevent base class.
 * This tevent is used to remenber a FLUSH has been sent
 * and take appropriate measures if no NACK are received
 * at timeout (e.g. free memory from list).
 */
class mcl_tx_ctrl_FLUSH_DONE_tevent : public mcl_tx_ctrl_tevent {

public:
	/****** Public Members ************************************************/

	/**
	 * Default constructor.
	 * @param adu_idf	ADU for which this FLUSH_DONE_tevent pertains
	 * @param block_idf	block within this ADU for which this
	 * 			FLUSH_DONE_tevent pertains
	 */
	mcl_tx_ctrl_FLUSH_DONE_tevent (UINT32		adu_idf,
				       UINT32		block_idf);

	/**
	 * Default destructor.
	 */
	virtual ~mcl_tx_ctrl_FLUSH_DONE_tevent ();

	/**
	 * Perform FLUSH_DONE_tevent specific processing.
	 * @return	true if tevent is completed and should be free'ed and
	 * 		removed from list, false if it has been re-scheduled.
	 */
	bool	process (mcl_cb		*const mclcb);

	/**
	 * Find a FLUSH_DONE_tevent in an unordered list with type-dependant
	 * criteria.
	 * Compares the adu_id and block_id arguments.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 * @return      returns a pointer to the tevent if found, NULL otherwise
	 */
	mcl_tx_ctrl_FLUSH_DONE_tevent  *find_in_list
						(mcl_cb		*const mclcb,
						 mcl_tx_ctrl_tevent	**head);

	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	UINT32				adu_id;	  // for this NORM_CMD_FLUSH
	UINT32				block_id; // for this NORM_CMD_FLUSH
	/**
	 * a FLUSH with no answer should be repeated for improved robustness.
	 * this is the nb of repeatitions remaining.
	 */
	UINT32				repeat_count;

};


/**
 * This class describes the RETX_DONE timer event.
 * It is derived from the mcl_tx_ctrl_tevent base class.
 * This tevent is used to register FLUSH events to send in the future.
 */
class mcl_tx_ctrl_RETX_DONE_tevent : public mcl_tx_ctrl_tevent {

public:
	/****** Public Members ************************************************/

	/**
	 * Default constructor.
	 * @param adu_idf	ADU for which this RETX_DONE_tevent pertains
	 * @param block_idf	block within this ADU for which this
	 * 			RETX_DONE_tevent pertains
	 */
	mcl_tx_ctrl_RETX_DONE_tevent (UINT32		adu_idf,
				       UINT32		block_idf);

	/**
	 * Default destructor.
	 */
	virtual ~mcl_tx_ctrl_RETX_DONE_tevent ();

	/**
	 * Perform RETX_DONE_tevent specific processing.
	 */
	void	process (mcl_cb		*const mclcb);

	/**
	 * Find a RETX_DONE_tevent in an unordered list with type-dependant
	 * criteria.
	 * Compares the adu_id and block_id arguments.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 * @return      returns a pointer to the tevent if found, NULL otherwise
	 */
	mcl_tx_ctrl_RETX_DONE_tevent  *find_in_list
						(mcl_cb		*const mclcb,
						 mcl_tx_ctrl_tevent	**head);


	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	UINT32				adu_id;	  // for this NORM_CMD_FLUSH
	UINT32				block_id; // for this NORM_CMD_FLUSH

};



//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#endif // !MCL_TX_CTRL_H
