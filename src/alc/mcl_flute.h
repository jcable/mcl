/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
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
#ifndef MCL_FLUTE_H
#define MCL_FLUTE_H


/**
 * This class provides all the required services to manage transmissions
 * in FLUTE mode.
 */
class mcl_tx_flute {
 
public:
	/****** Public Members ************************************************/

	/**
	 * Return the FDT instance ID to use for a new FDT instance.
	 * @return
	 */
	UINT32	get_next_FDT_instanceID ();

	/**
	 * Increment the FDT instance ID to use next time.
	 */
	void	incr_next_FDT_instanceID ();

	/****** Public Attributes *********************************************/

private:
	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/

	/* next FDT instance ID to deliver */
	UINT32	next_FDT_instanceID;
};


/**
 * Linked list of desired TOIs in FLUTE compatibility mode.
 */
typedef struct mcl_toi_list {
	struct mcl_toi_list	*next;
	UINT32 			toi;
} mcl_toi_list_t;


/**
 * This class provides all the required services to manage receptions
 * in FLUTE mode.
 */
class mcl_rx_flute {
 
public:
	/****** Public Members ************************************************/

	/** Constructor. */
	mcl_rx_flute ();

	/** Destructor. */
	~mcl_rx_flute ();

	/**
	 * Does the receiver use a FLUTE session that stores all ADUs by
	 * default?
	 * @return	boolean
	 */
	bool		use_flute_store_all_adus_by_default (void);

	/**
	 * Set the ADU delivery mode to the application.
	 * @param mclcb
	 * @param mode	delivery mode to set
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status  set_flute_store_all_adus_by_default
				(mcl_cb	*const mclcb,
				 bool	optvalue);

	/**
	 * Add a TOI to the list of TOIs requested by the FLUTE application.
	 * @param mclcb
	 * @param toi	TOI
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	add_requested_toi (class mcl_cb	*const mclcb,
						   UINT32		toi);

	/**
	 * Check if a particular TOI has been requested by the FLUTE
	 * application.
	 * @param mclcb
	 * @param toi	TOI
	 * @return	true if this TOI has been requested, false otherwise.
	 */
	bool 			is_toi_requested (class mcl_cb	*const mclcb,
						  UINT32		toi);


	/****** Public Attributes *********************************************/
	/* delete current packet ? */
	bool	delete_current_packet;

private:
	/****** Private Members ***********************************************/


	/****** Private Attributes ********************************************/

	/* Does the receiver use a FLUTE with storage of all adus by default. */
        bool      store_all_adus_by_default;


	/* list of desired TOIs in FLUTE_DELIVERY mode */
	struct mcl_toi_list		*desired_tois;

};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline UINT32
mcl_tx_flute::get_next_FDT_instanceID ()
{
	return this->next_FDT_instanceID;
}

inline void
mcl_tx_flute::incr_next_FDT_instanceID ()
{
	this->next_FDT_instanceID = 
		(this->next_FDT_instanceID + 1) % (2^24 - 1);
}

inline bool
mcl_rx_flute::use_flute_store_all_adus_by_default ()
{
	return (this->store_all_adus_by_default);
}

#endif /* !MCL_FLUTE_H */
