/* $Id: mcl_rx_thread.h,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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

#ifndef MCL_RX_THREAD_H
#define MCL_RX_THREAD_H


/**
 * Reception thread of a given MCL session.
 * This thread is in charge of listening to all reception sockets and
 * calling the appropriate processing function each time a new packet
 * arrives.
 */
class mcl_rx_thread {
public:
	/****** Public Members ************************************************/
	mcl_rx_thread ();
	~mcl_rx_thread ();	

	/**
	 * Starts the reception service.
	 * Exits in case of error.
	 */
	void	start (class mcl_cb *const mclcb);

	/**
	 * Return the thread id.
	 * Required to enable a safe thread destruction.
	 */
	mcl_thread_t	get_rx_thread_id ();

	/****** Public Attributes *********************************************/
  
	/**
	 * 
	 * @param XXX explanation
	 * @return Completion status (MCL_OK or MCL_ERROR).
	 */
  
private:
	/****** Private Members ***********************************************/

	/**
	 * Reception thread of the MCL session.
	 * This thread polls data regularly on the various mcast groups...
	 * Needs to be static since pthread_create() cannot be used with
	 * non-static member functions.
	 * @param arg	mclcb session pointer
	 */
	static void	*rx_thread (void *arg);


	/****** Private Attributes ********************************************/
	mcl_thread_t	rx_thread_id;	// idf returned at creation

};



//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline mcl_thread_t
mcl_rx_thread::get_rx_thread_id()
{
	return rx_thread_id;
}

#endif // MCL_RX_THREAD_H
