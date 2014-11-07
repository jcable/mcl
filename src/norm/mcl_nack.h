/* $Id: mcl_nack.h,v 1.2 2004/01/30 16:27:42 roca Exp $ */
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

#ifndef MCL_NACK_H
#define MCL_NACK_H

//
// NACK request object at a source
//
class src_nack_req {
	void	src_nack_req ();
	void	~src_nack_req ();
	// process a NACK request
	error_status	process ();
}


// status returned by update_loss_status method
enum loss_recovery_status = {
	LOSS_RECOVERY_STATUS_INVALID = 0,
	LOSS_RECOVERY_STATUS_NO,
	LOSS_RECOVERY_STATUS_YES,
	LOSS_RECOVERY_STATUS_PARTIAL
};

//
// NACK request object at a receiver
//
class recv_nack_req {
	void	recv_nack_req (loss_t *loss);
	void	~recv_nack_req ();
	// register a NACK for future transmission to source, if the
	// erasure is not recovered by that time...
	nack_req_id_t   register_nack_req (loss);
	// remove a registered NACK request (e.g. if the erasure has
	// been recovered in the mean-time)
	int     remove_nack_req ();
	// check before sending to source if the corresponding loss(es)
	// has(have) been recovered.
	// returns the new status.
	loss_recovery_status	update_loss_status ();
	// in case of LOSS_RECOVERY_STATUS_PARTIAL, update the NACK request
	// before processing it...
	error_status	update_nack_req ();

private:
	loss_descr	loss;	// describes the related loss or range of
				// losses for this NACK request
}



/******************************************************************************/


#ifdef NEVERDEF
//
// NACK processing at the source
//
class source_nack_proc {
public:
private:
}


// NACK request identifier in the receiver's NACK request list
typedef int	nack_req_id_t

//
// NACK processing at a receiver
//
class receiver_nack_proc {
public:
	void receiver_nack_proc (void);
	void ~receiver_nack_proc (void);
	// register a NACK for future transmission to source, if the
	// erasure is not recovered by that time...
	nack_req_id_t	register_nack_req (loss);
	// remove a registered NACK request (e.g. if the erasure has
	// been recovered in the mean-time)
	int	remove_nack_req (nack_req_id_t nack_id);
	//
	nack_req_id_t	find_nack_req (loss);

private:
}

#endif // NEVERDEF

#endif // MCL_NACK_H
