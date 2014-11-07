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

#include "mcl_includes.h"


/**
 * Constructor.
 */
mcl_rx_flute::mcl_rx_flute ()
{
	this->store_all_adus_by_default = 0;
	this->desired_tois = NULL;
	this->delete_current_packet = false;
}


/**
 * Destructor.
 */
mcl_rx_flute::~mcl_rx_flute ()
{
	mcl_toi_list_t	*e;	/* element */
	mcl_toi_list_t	*ne;	/* next element */

	/* there is no way to determine precisely if in FLUTE mode or not
	 * since there is no mclcb pointer... So be very carefull... */
	for (e = this->desired_tois; e != NULL; e = ne) {
		ne = e->next;
		free(e);
	}
	this->desired_tois = NULL;
}


/**
 * Set the delivery mode.
 * => See header file for more informations.
 */
mcl_error_status
mcl_rx_flute::set_flute_store_all_adus_by_default
				(mcl_cb	*const mclcb,
				 bool	optvalue)
{
	TRACELVL(5, (mcl_stdout,
		"-> mcl_cb::set_flute_store_all_adus_by_default: %d", optvalue))
	this->store_all_adus_by_default = optvalue;
	return MCL_OK;
}


/**
 * Add a TOI to the list of TOIs requested by the FLUTE application.
 */
mcl_error_status
mcl_rx_flute::add_requested_toi (mcl_cb		*const mclcb,
				 UINT32		toi)
{
	mcl_toi_list_t	*newtoi;

	ASSERT(mclcb->is_flute_compatible());
	if ((newtoi = (mcl_toi_list_t *)malloc(sizeof(mcl_toi_list_t)))
				== NULL) {
		PRINT_ERR((mcl_stderr,
			"mcl_flute_add_requested_toi: ERROR, out of memory\n"))
		return MCL_ERROR;
	}
	newtoi->toi = toi;
	newtoi->next = this->desired_tois;
	this->desired_tois = newtoi;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_flute_add_requested_toi: TOI=%d added\n", toi))
	return MCL_OK;
}


/**
 * Check if a particular TOI has been requested by the FLUTE
 * application.
 */
bool
mcl_rx_flute::is_toi_requested (mcl_cb	*const mclcb,
				UINT32	toi)
{
	bool		found = false;
	mcl_toi_list_t	*listloop = this->desired_tois;

	ASSERT(mclcb->is_flute_compatible());
	if (toi == 0) {
		return 1;
	}
	while (listloop != NULL) {
		if (listloop->toi == toi) {
			found = true;
			break;
		}
		listloop = listloop->next;
	}
	TRACELVL(5, (mcl_stdout,
		"<- mcl_flute_is_toi_requested: TOI=%d is %s\n",
		toi, found ? "req" : "not req"))
	return found;
}


