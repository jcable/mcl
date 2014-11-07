/* $Id: mcl_vrmem.h,v 1.6 2005/05/17 12:36:59 roca Exp $ */
/*
 *  Copyright (c) 2005 INRIA - All rights reserved
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

/*
 * "virtual receive memory service"
 *
 * This file implements a "virtual memory service" where incoming data
 * (ie. data and FEC DUs received or decoded) is stored either in memory
 * and/or on disk, so that at any time the total size of buffers (ie. data
 * in memory) does not exceed a certain threshold.
 *
 * TERMINOLOGY:
 * - vrmem	virtual receive memory service: this is the service
 * - vrbuf	vrmem buffer: this is the vrmem set of buffers, in RAM
 *		The total size of buffers is of course limited.
 * - vrfile	vrmem file: this is the vrmem big file, on the disk
 *
 * LIMITATIONS:
 * - single very large file, located by default in /tmp. Don't forget to
 *   change default location if not enough room there.
 * - there is no possibility to remove a part of the file (eg. once an ADU
 *   has been completely received and returned to the application).
 *   The only possibility is to remove the whole file!
 */


#ifndef MCL_VRMEM_H  /* { */
#define MCL_VRMEM_H

#ifdef VIRTUAL_RX_MEM


/**
 * Storage status of a DU.
 * Associated FSM:
 *
 *                        register_in_vrbuf
 *           INVALID --------------------------> ONLY_IN_VRBUF
 *             ^|    <--------------------------      |
 *             ||         unregister_from_vrbuf       |
 *             ||                                     +
 * remove_from || store_in_vrfile                    /
 *     _vrfile ||                                   /  
 *             ||                                  / store_in_vrfile  
 *             ||+--------------------------------+    
 *             |||                                     
 *             |VV        get_from_vrfile              
 *       ONLY_IN_VRFILE -----------------------> IN_VRFILE_AND_VRBUF
 *                      <-----------------------      |         ^
 *                        unregister_from_vrbuf       |         |
 *                                                    +---------+
 *                                          update_vrfile_from_dirty_vrbuf
 */
enum mcl_vrmem_storage_status {
	STORAGE_STATUS_INVALID = 0,	/* nothing stored */
	STORAGE_STATUS_ONLY_IN_VRBUF,	/* only in memory buffer */
	STORAGE_STATUS_ONLY_IN_VRFILE,	/* only in file, nothing in mem buf */
	STORAGE_STATUS_IN_VRFILE_AND_VRBUF /* permanently stored in vrfile */
					/* with a copy available in mem buf */
};


/**
 * DU <=> vrmem correspondance.
 * Stored in the du_t structure.
 */
typedef struct vrmem_info {
	mcl_vrmem_storage_status  status;	/* DU storage status */
	INT64			off;		/* DU offset in vrfile. Only */
						/* valid in *_VRFILE_* states */
	struct du		*prev_in_vrbuf; /* Pointer to previous DU in */
						/* *_IN_VRBUF_* state */
	struct du		*next_in_vrbuf; /* Pointer to next DU in */
						/* *_IN_VRBUF_* state */
} vrmem_info_t;


#if 0
/*
 * Describes each buffer of the vrbuf system.
 * Managed as a single table of vrmem_buf_entry_t structs in practice.
 * The buffer associated to a DU is the mcl_rx_pkt object, du->pkt.
 */
typedef struct {
	//char		*data;		/* data block of this entry */
	struct du	*du;		/* owner of this entry */
} vrmem_buf_entry_t;
#endif


/**
 * This Class implements the Virtual Receive Memory service.
 */
class mcl_vrmem {
 
public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	mcl_vrmem ();

	/**
	 * Default destructor.
	 */
	~mcl_vrmem ();

	/**
	 * Frees the vrmem file and closes the service.
	 * @param mclcb
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	close	(class mcl_cb *const mclcb);

	/**
	 * Must we use the vrmem file to actually store data?
	 * This is only possible if the vrmem service is used and there's
	 * no room any more in the vrmem buffer.
	 * @param mclcb
	 * @param len		data size.
	 * @return		true if vrfile must be used, false if it is
	 *			possible to use the vrbuf.
	 */
	bool	must_store_in_vrfile	(class mcl_cb	*const mclcb,
					 UINT32		len);

	/**
	 * Stores data of the DU in the vrmem file, unregisters (if
	 * needed) and free's the buffer.
	 * Data is stored in arrival order in vrfile order to maximize the
	 * write-to-disk speed.
	 * Data is assumed to be in du->data and du->pkt, but this buffer
	 * will be free'd by this function. Even if data is in du->data,
	 * the state upon calling this function can either be INVALID
	 * (ie. not registered in vrbuf) or ONLY_IN_VRBUF (ie. already
	 * registered in vrbuf).
	 * @param mclcb
	 * @param du		DU associated to the data buffer.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	store_in_vrfile
					(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * The vrbuf copy is newer than the vrfile copy, so update
	 * the permanent repository (similar to a cache entry with
	 * the "dirty bit" set).
	 * Only valid in IN_VRFILE_AND_VRBUF state, does nothing in
	 * other states.
	 * @param mclcb
	 * @param du		DU associated to the dirty data buffer.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	update_vrfile_from_dirty_vrbuf
					(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * Retrieves data from disk for this DU and copies it to the
	 * buffer pointed by du->data.
	 * A rx_pkt buffer must already be allocated, du->pkt and du->data
	 * must be set accordingly.
	 * @param mclcb
	 * @param du		DU associated to the data buffer.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	get_from_vrfile	
					(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * Removes data from disk for this DU.
	 * @param mclcb
	 * @param du		DU associated to the data buffer.
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	remove_from_vrfile
					(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * Register this DU in the vrmem buffer system.
	 * There is no additional data copy nor buffer allocation, this
	 * function only takes into account that len additional bytes are
	 * stored in RAM.
	 * Used later on by must_store_in_vrfile() to know if new data
	 * packets can be kept in RAM or must be stored in vrmem file.
	 * @param mclcb
	 * @param du		DU associated to the buffer.
	 */
	void	register_in_vrbuf	(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * Removes this DU from the vrmem buffer system, and free's
	 * the associated buffer.
	 * @param mclcb
	 */
	void	remove_from_vrbuf	(class mcl_cb	*const mclcb,
					 struct du	*du);

	/**
	 * Returns true if this DU is stored in the vrfile.
	 * @param mclcb
	 * @return		true if data is in vrfile.
	 */
	bool	in_vrfile	(class mcl_cb	*const mclcb,
				 struct du	*du);

	/**
	 * Returns true if this DU is stored in the vrbuf.
	 * @param mclcb
	 * @return		true if data is in vrbuf.
	 */
	bool	in_vrbuf	(class mcl_cb	*const mclcb,
				 struct du	*du);


	/****** Public Attributes *********************************************/


private:
	/****** Private Members ***********************************************/

	/**
	 * Finishes the initialization of the vrmem service.
	 * The vrmem file is now allocated.
	 * This call should happen only when one is about to use the vrmem
	 * file to avoid extra processing.
	 * @param mclcb
	 * @return		Completion status (MCL_OK or MCL_ERROR).
	 */
	mcl_error_status	finish_init (class mcl_cb *const mclcb);


	/**
	 * Add this DU to the end of the list of DUs in _IN_VRBUF_ state.
	 * Performs only the various pointer updates.
	 * @param mclcb
	 * @param du		DU to add at the end of list.
	 */
	void	add_to_end_of_vrbuf_list	(class mcl_cb	*const mclcb,
						 struct du	*du);

	/**
	 * Remove this DU from the list of DUs in _IN_VRBUF_ state.
	 * Performs only the various pointer updates.
	 * @param mclcb
	 * @param du		DU to remove.
	 */
	void	remove_from_vrbuf_list	(class mcl_cb	*const mclcb,
						 struct du	*du);


	/****** Private Attributes ********************************************/

	/**
	 * True once the vrmem service has been fully initialized, and
	 * in particular the vrmem file created.
	 * This happens once initialize() has been called.
	 */
	bool	initialized;

	/*
	 * vrmem file info.
	 */
	/** vrmem file name. */
	char		vrfile_name[MAX_FILE_NAME_LEN];
	/** vrmem file fd. */
	INT32		vrfile_fd;
	/** current vrmem file length. */
	INT64		vrfile_len;

	/*
	 * vrmem buffer info.
	 */
	/** Maximum vrmem buffer size possible before using the vrmem file. */
	INT64		vrbuf_max_size;
	/** Total physical receive memory buffer allocated. */
	INT64		vrbuf_size;
	/**
	 * Start of the doubly linked list of DUs with _IN_VRBUF_ state.
	 * This list is used to swap to disk some DUs when room is needed
	 * by a get_from_vrfile() call.
	 */
	struct du	*in_vrbuf_list_head;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------

#if 0
inline bool
mcl_vrmem::on_disk (class mcl_cb	*const mclcb,
		    struct du		*du)
{
	ASSERT(du);
	//TRACELVL(5, (mcl_stdout, "   mcl_vrmem::on_disk: %d\n", du->vrm_info.on_disk))
printf("%d\n", du->len);
	//return du->vxm.r_info_u.on_disk;
	return du->vrm_info.on_disk;
}
#endif


#endif /* VIRTUAL_RX_MEM */

#endif /* }  MCL_VRMEM_H */
