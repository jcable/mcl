/* $Id: mcl_rx_ctrl.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_RX_CTRL_H
#define MCL_RX_CTRL_H


/**
 * Receiver's control tasks.
 * This class is used for NORM control message reception and transmission
 * (if applicable) at a receiver.
 * This class inherits the timer_handler class in order to have a timer
 * callback function.
 */
class mcl_rx_ctrl : public mcl_timer_handler {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	mcl_rx_ctrl();

	/**
	 * Process an incoming NORM_CMD packet.
	 * @param mclcb
	 * @param pkt		pointer to packet
	 * @param saddr		source address for this packet
	 * @param chdr_infos	pointer to the common_infos struct
	 */
	mcl_error_status  process_cmd_pkt (mcl_cb		*const mclcb,
					   class mcl_rx_pkt	*pkt,
					   mcl_addr		*saddr,
					   mcl_common_hdr_infos_t *chdr_infos);

	/**
	 * timer callback method.
	 * @param arg1		mclcb
	 * @param arg2		pointer to an rx_ctrl_timer_event_t struct
	 */
	virtual void timer_callback (INT32 arg1, INT32 arg2);


	/****** Public Attributes *********************************************/

	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
 
protected:
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/

	/**
	 * Create a list of pending timer_events.
	 * Required for instance to avoid duplicated timer_events, when
	 * receiving several instances of the same FLUSH command.
	 * There is one list per tevent type.
	 */
	class mcl_rx_ctrl_tevent	*NACK_tevent_head;
	//class mcl_rx_ctrl_tevent	*ACK_tevent_head;
 
};


/**
 * Types of timer events.
 */
enum mcl_rx_ctrl_tevent_types {
	RX_CTRL_TEVENT_INVALID = 0,
	RX_CTRL_TEVENT_START_NACK_PROCESS,
	RX_CTRL_TEVENT_START_ACK_PROCESS
};


/**
 * This base class describes timer events.
 * This is the second arg of timer_callback() method.
 */
class mcl_rx_ctrl_tevent {

public:
	/****** Public Members ************************************************/

#if 0
	/**
	 * Default constructor.
	 */
	mcl_rx_ctrl_tevent () = 0;
#endif

	/**
	 * Default destructor.
	 */
	virtual	~mcl_rx_ctrl_tevent ();

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
					  mcl_rx_ctrl_tevent	**head);

	/**
	 * Remove a tevent from an unordered list.
	 * Called with the appropriate list head parameter.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 */
	mcl_error_status  remove_from_list (mcl_cb		*const mclcb,
					    mcl_rx_ctrl_tevent	**head);


	/****** Public Attributes *********************************************/

	/** Control message type. */
	mcl_rx_ctrl_tevent_types	type;


protected:
	/****** Protected Members *********************************************/
	/****** Protected Attributes ******************************************/
	mcl_rx_ctrl_tevent	*prev, *next;


private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/

};


/**
 * This class describes the NACK timer event.
 * It is derived from the mcl_rx_ctrl_tevent base class.
 */
class mcl_rx_ctrl_NACK_tevent : public mcl_rx_ctrl_tevent {

public:
	/****** Public Members ************************************************/

	/**
	 * Default constructor.
	 * @param adu_idf	ADU for which this NACK_tevent pertains
	 * @param block_idf	block within this ADU for which this
	 * 			NACK_tevent pertains
	 */
	mcl_rx_ctrl_NACK_tevent (UINT32		adu_idf,
				 UINT32		block_idf,
				 mcl_addr	*saddr);

	/**
	 * Default destructor.
	 */
	virtual ~mcl_rx_ctrl_NACK_tevent ();

	/**
	 * Perform NACK_tevent specific processing.
	 */
	void	process (mcl_cb		*const mclcb);

	/**
	 * Find a NACK_tevent in an unordered list with type-dependant criteria.
	 * Compares the adu_id and block_id arguments.
	 * @param mclcb
	 * @param head  pointer to the list head pointer
	 * @return      returns a pointer to the tevent if found, NULL otherwise
	 */
	mcl_rx_ctrl_NACK_tevent  *find_in_list (mcl_cb		*const mclcb,
						mcl_rx_ctrl_tevent	**head);


	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
	UINT32				adu_id;	  // used with NORM_CMD_FLUSH
	UINT32				block_id; // used with NORM_CMD_FLUSH
	/**
	 * Address of the sender of a control message.
	 * It is sometimes required to have it directly, e.g. if a ctrl
	 * msg is received for an ADU for which we didn't receive any
	 * packet yet (e.g. they have all been lost)
	 */
	mcl_addr			src_addr;

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


#if 0
inline mcl_error_status 
mcl_rx_ctrl_NACK_tevent::insert_in_list (mcl_cb		*const mclcb)
{
	return mcl_rx_ctrl_tevent::insert_in_list(mclcb,
					&(mclcb->rx_ctrl->NACK_tevent_head));
}

inline mcl_error_status 
mcl_rx_ctrl_NACK_tevent::remove_from_list (mcl_cb		*const mclcb)
{
	return mcl_rx_ctrl_tevent::insert_in_list(mclcb,
			  &((mcl_rx_ctrl_tevent*)this->NACK_tevent_head));
}

inline mcl_rx_ctrl_NACK_tevent  *
mcl_rx_ctrl_NACK_tevent::find_in_list (mcl_cb		*const mclcb)
{
	return mcl_rx_ctrl_tevent::insert_in_list(mclcb,
			  &((mcl_rx_ctrl_tevent*)this->NACK_tevent_head));
}
#endif

#endif // MCL_RX_CTRL_H
