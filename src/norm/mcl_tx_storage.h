/* $Id: mcl_tx_storage.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_TX_STORAGE_H
#define MCL_TX_STORAGE_H

/**
 * Class explanation
 */
class mcl_tx_storage {
 
public:
	/****** Public Members ************************************************/
	mcl_tx_storage ();
	~mcl_tx_storage ();

	/**
	 * store data buffer in a private internal tx buffer and update adu ptr.
	 * @param mclcb
	 * @param adu	adu to which buffer belongs
	 * @param buf	buffer whose content needs to be stored in a permanent
	 * 		location (typically this buf is allocated by the upper
	 * 		application)
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	store_adu_data (mcl_cb		*const mclcb,
						mcl_adu		*const adu,
						char		*const buf);

	/**
	 * store data buffer in a private internal tx buffer and update du ptr.
	 * @param mclcb
	 * @param du	du to which buffer belongs
	 * @param buf	buffer whose content needs to be stored in a permanent
	 * 		location
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	store_du_data (mcl_cb		*const mclcb,
						mcl_du		*const du,
						char		*const buf);

	/****** Public Attributes *********************************************/
 
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
 
private:
	/****** Private Members ***********************************************/
	/****** Private Attributes ********************************************/
 
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#endif // !MCL_TX_STORAGE_H
