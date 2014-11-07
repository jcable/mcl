/* $Id: mcl_rx_storage.h,v 1.5 2005/03/23 14:05:03 roca Exp $ */
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

#ifndef MCL_RX_STORAGE
#define MCL_RX_STORAGE


/**
 * Class that stores all the incoming packets.
 */
class mcl_rx_pkt {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 * Allocates the data buffer with the size provided.
	 * @param max_size	the maximum packet size, including headers
	 */
	mcl_rx_pkt (INT32		max_size);

	/**
	 * Default destructor.
	 * This destructor deletes both the pkt and the buffer, but NOT the DU
	 * (if any).
	 */
	~mcl_rx_pkt ();

	/**
	 * Return a pointer to the buffer where the whole packet is stored.
	 * @return	pointer to buffer.
	 */
	char	*get_buf ();

	/**
	 * Return the buffer length.
	 * The actual size of the packet stored is lower or equal to this
	 * buffer size.
	 * @return	buffer length.
	 */
	INT32	get_buf_len ();

	/****** Public Attributes *********************************************/

	/**
	 * Return the whole packet length, including headers.
	 */
	INT32		pkt_len;


private:
	/****** Private Members ***********************************************/

	/****** Private Attributes ********************************************/

	/**
	 * Buffer where packet is stored.
	 * This buffer is allocated once, at creation.
	 */
	char	*buf;

	/**
	 * Length of this buffer.
	 */
	INT32	buf_len;
};


/**
 * Class that offers a dedicated storage service.
 */
class mcl_rx_storage {
//friend class mcl_rx_pkt;	/* needed for rx_pkt to access some vrmem */
				/* methods */
 
public:
	/****** Public Members ************************************************/

	/**
	 * Default constructor.
	 */
	mcl_rx_storage();

	/**
	 * Default destructor.
	 */
	~mcl_rx_storage();

	/**
	 * Stores data of the DU permanently, either in RAM or in the
	 * vrmem file, whichever is the most appropriate.
	 * The caller must have stored data in an rx_pkt class, associated
	 * to the DU. This function then either keeps data there, if enough
	 * space in RAM, or stores it in the vrfile and frees the rx_pkt
	 * class.
	 * @param mclcb
	 * @param du	associated DU.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	store_data (class mcl_cb	*const mclcb,
					    du_t		*du);

#if 0
	/**
	 * Stores data of the DU permanently, either in RAM or in the
	 * vrmem file, whichever is the most appropriate.
	 * The caller must have stored data in an rx_pkt class, associated
	 * to the DU. This function then either keeps data there, if enough
	 * space in RAM, or stores it in the vrfile and frees the rx_pkt
	 * class.
	 * @param mclcb
	 * @param du	associated DU.
	 * @param data	data of the DU that must be stored.
	 * @param len	data length.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	store_data (class mcl_cb	*const mclcb,
					    du_t		*du,
					    char		*data,
					    UINT32		len);
#endif

	/**
	 * Copies data of the DU in the buffer given in parameter, either
	 * from a source data buffer or from the vrmem file, whichever
	 * is appropriate.
	 * When needed, a new rx_pkt will be allocated and du->data set
	 * accordingly.
	 * @param mclcb
	 * @param du	associated DU.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	get_data (class mcl_cb		*const mclcb,
					  du_t			*du);

#if 0
	/**
	 * Copies data of the DU in the buffer given in parameter, either
	 * from a source data buffer or from the vrmem file, whichever
	 * is appropriate.
	 * @param mclcb
	 * @param du	associated DU.
	 * @param buf	destination buffer.
			If NULL, a new rx_pkt will be allocated and du->data
			set accordingly.
	 * @param len	data length.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	get_data (class mcl_cb		*const mclcb,
					  du_t			*du,
					  char			*data,
					  UINT32		len);
#endif

	/**
	 * Unregisters data of the DU and frees the associated data buffer.
	 * @param mclcb
	 * @param du	associated DU.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	free_data (class mcl_cb	*const mclcb,
					   du_t		*du);

#ifdef VIRTUAL_RX_MEM
	/**
	 * Turns on or off the Virtual receive memory service.
	 * @param mclcb
	 * @param set		true if vrmem is desired, false otherwise.
	 */
	void	set_vrmem	(class mcl_cb	*const mclcb,
				 bool		set);

	/**
	 * Is the Virtual receive memory service used?
	 * This does not mean that data has actually been stored in the
	 * vrmem file itself, just that it can be done if needed.
	 * @return	true if the service is used, false otherwise.
	 */
	bool	is_vrmem_used	();

	/**
	 * Closes the Virtual receive memory service, when applicable.
	 * @param mclcb
	 */
	void	close_vrmem	(class mcl_cb	*const mclcb);

	/**
	 * Returns true if this DU is stored in the vrbuf.
	 * Needed for statistics purposes.
	 * @param mclcb
	 * @return		true if data is in vrbuf.
	 */
	bool	in_vrbuf	(class mcl_cb	*const mclcb,
				 struct du	*du);
#endif
 

	/****** Public Attributes *********************************************/
  
	/**
	 * 
	 * @param XXX explanation
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
 

private:
	/****** Private Members ***********************************************/

	/**
	 * Initialize the Virtual receive memory service.
	 * @return	Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	initialize_vrmem ();


	/****** Private Attributes ********************************************/

	/** Is the Virtual receive memory service is desired or not. */
	bool			vrmem_desired;

	/**
	 * Indicates if the Virtual receive memory service is used or not,
	 * i.e. if the mcl_vrmem class has been allocated and initialized.
	 */
	bool			vrmem_used;

#ifdef VIRTUAL_RX_MEM
	/** Virtual receive memory service class.
	 * Allocated/initialized only if vrmem_used is true. */
	class mcl_vrmem		*vrmem;
#endif
 
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

inline char *
mcl_rx_pkt::get_buf ()
{
	return this->buf;
}

inline INT32
mcl_rx_pkt::get_buf_len ()
{
	return this->buf_len;
}

#ifdef VIRTUAL_RX_MEM
inline void
mcl_rx_storage::set_vrmem	(class mcl_cb	*const mclcb,
				 bool		set)
{
	this->vrmem_desired = set;
}

inline bool
mcl_rx_storage::is_vrmem_used ()
{
	return this->vrmem_used;
}

inline bool
mcl_rx_storage::in_vrbuf	(class mcl_cb	*const mclcb,
				 struct du	*du)
{
	return this->vrmem->in_vrbuf(mclcb, du);
}

#endif

#endif // MCL_RX_STORAGE
