/* $Id: mcl_vrmem.cpp,v 1.8 2005/05/17 12:36:58 roca Exp $ */
/*
 *  Copyright (c) 1999-2004 INRIA - All rights reserved
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
 * This file implements a "virtual memory service" where received data
 * (ie. data and FEC DUs received) is stored on disk.
 * Shared by all mcl contexts.
 * Data in physical memory is a "read-only" version of data on file.
 *
 * LIMITATIONS:
 * - single very large file, located by default in /tmp. don't forget to
 *   change default location if not enough room there.
 * - there is no possibility to remove a part of the file (eg. once an ADU
 *   has been completely received and returned to the application).
 *   The only possibility is to remove the whole file!
 */

#include "mcl_includes.h"


#ifdef VIRTUAL_RX_MEM

/****** Public Functions ******************************************************/

/**
 * Constructor.
 * => See header file for more informations.
 */
mcl_vrmem::mcl_vrmem ()
{
	/* not fully initialized yet, in particular the vrmem file has
	 * not yet been allocated... */
	this->initialized = false;
	memset(this->vrfile_name, 0, sizeof(this->vrfile_name));
	this->vrfile_fd = 0;
	this->vrfile_len = 0;
	this->vrbuf_max_size = VIRTUAL_RX_MEM_MAX_BUFFERING_SIZE;
	this->vrbuf_size = 0;
	this->in_vrbuf_list_head = NULL;
}


/**
 * Destructor.
 * => See header file for more informations.
 */
mcl_vrmem::~mcl_vrmem ()
{
	if (this->initialized)
		this->close(NULL);
	this->initialized = false;
	memset(this->vrfile_name, 0, sizeof(this->vrfile_name));
	this->vrfile_fd = 0;
	this->vrfile_len = 0;
	this->vrbuf_max_size = VIRTUAL_RX_MEM_MAX_BUFFERING_SIZE;
	this->vrbuf_size = 0;
	this->in_vrbuf_list_head = NULL;
}


/**
 * => See header file for more informations.
 */
mcl_error_status
mcl_vrmem::close (mcl_cb	*const mclcb)
{
	if (!this->initialized)
		return MCL_OK;
	if (mclcb)
		TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::close:\n"))
	this->initialized = false;
	unlink(this->vrfile_name);	/* remove vrmem file */
	this->vrfile_fd = 0;
	if (mclcb)
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::close: ok\n"))
	return MCL_OK;
}


/**
 * => See header file for more informations.
 */
bool
mcl_vrmem::must_store_in_vrfile (mcl_cb		*const mclcb,
				 UINT32		len)
{
	//ASSERT(mclcb->rx_storage->is_vrmem_used() == true);
	TRACELVL(5, (mcl_stdout,
		"   mcl_vrmem::must_store_in_vrfile: vrbuf usage: sz=%Ld, max_sz=%Ld, want to store %d\n",
		this->vrbuf_size, this->vrbuf_max_size, len))
	if (this->vrbuf_size + len <= this->vrbuf_max_size) {
		TRACELVL(5, (mcl_stdout,
		"<- mcl_vrmem::must_store_in_vrfile: no, there's room in vrbuf\n"))
		return false;
	} else {
		TRACELVL(5, (mcl_stdout,
			"<- mcl_vrmem::must_store_in_vrfile: yes\n"))
		return true;
	}
}


/**
 * => See header file for more informations.
 */
mcl_error_status
mcl_vrmem::store_in_vrfile (mcl_cb	*const mclcb,
			    du_t	*du)
{
	INT64	off;		/* offset */
	INT32	err;

	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::store_in_vrfile:\n"))
	//ASSERT(mclcb->rx_storage->is_vrmem_used() == true);
	if (!this->initialized)
		this->finish_init(mclcb);
	ASSERT(du);
	ASSERT(du->pkt);
	ASSERT(du->data);
	ASSERT(du->len > 0);
	if (du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRBUF) {
		/* unregister from vrbuf */
		this->vrbuf_size -= du->len;
		ASSERT(this->vrbuf_size >= 0);
		/* remove this DU from the list of DUs in _IN_VRBUF_ state */
		this->remove_from_vrbuf_list (mclcb, du);
	} else {
		ASSERT(du->vrm_info.status == STORAGE_STATUS_INVALID);
	}
	du->vrm_info.status = STORAGE_STATUS_ONLY_IN_VRFILE;
	off = du->vrm_info.off = this->vrfile_len;
	this->vrfile_len += du->len;
	/* repositionning within file is absolutely required! */
	if (mcl_file_lseek(this->vrfile_fd, off) < 0) {
		perror("mcl_vrmem::store_in_vrfile: ERROR: lseek failed\n");
		goto error;
	}
	/* store data */
	if ((err = mcl_file_write(this->vrfile_fd, du->data, du->len)) < 0) {
		perror("mcl_vrmem::store_in_vrfile: ERROR: write failed\n");
		goto error;
	}
	/* remove buffer now */
	delete du->pkt;
	du->pkt = NULL;
	du->data = NULL;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_vrmem::store_in_vrfile: starts at off=%Ld\n", off))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::store_in_vrfile: ERROR\n"))
	return MCL_ERROR;
}


/**
 * The vrbuf copy is newer than the vrfile copy, so update
 * the permanent repository (similar to a cache entry with
 * the "dirty bit" set).
 * => See header file for more informations.
 */
mcl_error_status
mcl_vrmem::update_vrfile_from_dirty_vrbuf (class mcl_cb	*const mclcb,
					   struct du	*du)
{
	INT64	off;		/* offset */
	INT32	err;

	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::update_vrfile_from_dirty_vrbuf:\n"))
	ASSERT(this->initialized);
	ASSERT(du);
	ASSERT(du->pkt);
	ASSERT(du->data);
	ASSERT(du->len > 0);
	if (du->vrm_info.status != STORAGE_STATUS_IN_VRFILE_AND_VRBUF) {
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::update_vrfile_from_dirty_vrbuf: nothing to do, bad status (%d)\n", du->vrm_info.status))
		return MCL_OK;
	}
	off = du->vrm_info.off;
	/* repositionning within file is absolutely required! */
	if (mcl_file_lseek(this->vrfile_fd, off) < 0) {
		perror("mcl_vrmem::update_vrfile_from_dirty_vrbuf: ERROR: lseek failed\n");
		goto error;
	}
	/* store data */
	if ((err = mcl_file_write(this->vrfile_fd, du->data, du->len)) < 0) {
		perror("mcl_vrmem::update_vrfile_from_dirty_vrbuf: ERROR: write failed\n");
		goto error;
	}
	TRACELVL(5, (mcl_stdout,
		"<- mcl_vrmem::update_vrfile_from_dirty_vrbuf:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::update_vrfile_from_dirty_vrbuf: ERROR\n"))
	return MCL_ERROR;

}


/**
 * => See header file for more informations.
 */
mcl_error_status
mcl_vrmem::get_from_vrfile (mcl_cb	*const mclcb,
			    du_t	*du)
{
	UINT32	len;

	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::get_from_vrfile:\n"))
	//ASSERT(mclcb->rx_storage->is_vrmem_used() == true);
	ASSERT(this->initialized);
	ASSERT(du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRFILE);
	/* remove as many buffers already allocated (usually one is enough), 
	 * as long as vrbuf_size + len > max_vrbuf_size */
	while (this->vrbuf_size + du->len > this->vrbuf_max_size) {
		du_t	*this_one = this->in_vrbuf_list_head;
		if (this_one->vrm_info.status ==
					STORAGE_STATUS_IN_VRFILE_AND_VRBUF) {
			this->remove_from_vrbuf(mclcb, this_one);
		} else {
			ASSERT(this_one->vrm_info.status ==
					STORAGE_STATUS_ONLY_IN_VRBUF);
			this->store_in_vrfile(mclcb, this_one);
		}
		/* update stats */
		mclcb->stats.buf_space -= this_one->len;
	}
	/* update the vrbuf list size */
	this->vrbuf_size += du->len;
	/* update the storage status */
	du->vrm_info.status = STORAGE_STATUS_IN_VRFILE_AND_VRBUF;
	/* add this DU to the list of DUs in _IN_VRBUF_ state */
	this->add_to_end_of_vrbuf_list(mclcb, du);
	/* and read data from disk */
	/* repositionning within file is absolutely required! */
	if (mcl_file_lseek(this->vrfile_fd, du->vrm_info.off) < 0) {
		perror("mcl_vrmem::get_from_vrfile: ERROR: lseek failed\n");
		goto error;
	}
	if ((len = mcl_file_read(this->vrfile_fd, du->data, du->len)) <= 0 ||
	     len != du->len) {
		perror("mcl_vrmem::get_from_vrfile: ERROR: read failed\n");
		goto error;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::get_from_vrfile:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::get_from_vrfile: ERROR\n"))
	return MCL_ERROR;
}


/**
 * Removes data from disk for this DU.
 */
mcl_error_status
mcl_vrmem::remove_from_vrfile  (class mcl_cb	*const mclcb,
				struct du	*du)
{
	ASSERT(du);
	TRACELVL(5, (mcl_stdout, "   mcl_vrmem::remove_from_vrfile:\n"))
	ASSERT(du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRFILE);
	du->vrm_info.status = STORAGE_STATUS_INVALID;
	return MCL_OK;
}


/**
 * => See header file for more informations.
 */
void
mcl_vrmem::register_in_vrbuf (mcl_cb	*const mclcb,
			      du_t	*du)
{
	ASSERT(du);
	ASSERT(du->len > 0);
	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::register_in_vrbuf: len=%d\n",
		du->len))
	/* update the vrbuf list size */
	this->vrbuf_size += du->len;
	//ASSERT(this->vrbuf_size <= this->vrbuf_max_size);
	/* update the storage status */
	ASSERT(du->vrm_info.status == STORAGE_STATUS_INVALID);
	du->vrm_info.status = STORAGE_STATUS_ONLY_IN_VRBUF;
	/* add this DU to the list of DUs in _IN_VRBUF_ state */
	this->add_to_end_of_vrbuf_list(mclcb, du);
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::register_in_vrbuf:\n"))
}


/**
 * => See header file for more informations.
 */
void
mcl_vrmem::remove_from_vrbuf (mcl_cb	*const mclcb,
				  du_t		*du)
{
	ASSERT(du);
	ASSERT(du->len > 0);
	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::remove_from_vrbuf: len=%d\n",
		du->len))
	/* remove this DU from the list of DUs in _IN_VRBUF_ state */
	this->remove_from_vrbuf_list (mclcb, du);
	/* update the vrbuf list size */
	this->vrbuf_size -= du->len;
	ASSERT(this->vrbuf_size >= 0);
	/* update the storage status */
	if (du->vrm_info.status == STORAGE_STATUS_IN_VRFILE_AND_VRBUF) {
		du->vrm_info.status = STORAGE_STATUS_ONLY_IN_VRFILE;
	} else {
		ASSERT(du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRBUF);
		du->vrm_info.status = STORAGE_STATUS_INVALID;
	}
	/* remove buffer now */
	ASSERT(du->pkt);
	delete du->pkt;
	du->pkt = NULL;
	du->data = NULL;
	TRACELVL(5, (mcl_stdout,
		"<- mcl_vrmem::remove_from_vrbuf: remains %Ld\n",
		this->vrbuf_size))
}


/**
 * => See header file for more informations.
 */
bool
mcl_vrmem::in_vrfile (class mcl_cb	*const mclcb,
		      du_t		*du)
{
	ASSERT(du);
	if ((du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRFILE) ||
	    (du->vrm_info.status == STORAGE_STATUS_IN_VRFILE_AND_VRBUF)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::in_vrfile: yes\n"))
		return true;
	} else {
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::in_vrfile: no\n"))
		return false;
	}
}


/**
 * => See header file for more informations.
 */
bool
mcl_vrmem::in_vrbuf	(class mcl_cb	*const mclcb,
			 struct du	*du)
{
	ASSERT(du);
	if ((du->vrm_info.status == STORAGE_STATUS_ONLY_IN_VRBUF) ||
	    (du->vrm_info.status == STORAGE_STATUS_IN_VRFILE_AND_VRBUF)) {
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::in_vrbuf: yes\n"))
		return true;
	} else {
		TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::in_vrbuf: no\n"))
		return false;
	}
}


/****** Private Functions *****************************************************/


/**
 * Finish the initialization.
 * => See header file for more informations.
 */
mcl_error_status
mcl_vrmem::finish_init (mcl_cb	*const mclcb)
{
	TRACELVL(5, (mcl_stdout, "-> mcl_vrmem::finish_init:\n"))
	ASSERT(this->initialized == false);
#if 0
	vcb->entry_size = mclcb->datagram_sz;	/* fixed size! */
	if (!(pm_tab = (vrm_cache_entry_t*)calloc(mcl_vrm_cache_entry_nb,
						 sizeof(vrm_cache_entry_t))) ||
	    !(pm_blk = (char*)malloc(mcl_vrm_cache_entry_nb * vcb->entry_size)))
	{
		PRINT_ERR((mcl_stderr,"mcl_vrmem::finish_init: ERROR: out of memory"))
		goto error;
	}
	for (i = 0, pm = pm_tab, off = 0;
	     i < (int)mcl_vrm_cache_entry_nb;
	     i++, pm++, off += vcb->entry_size) {
		pm->data = pm_blk + off;
	}
	vcb->entry_nb = mcl_vrm_cache_entry_nb;
	vcb->pmem_tab = pm_tab;
	vcb->next_index = 0;
#endif

	/* create temp file now in the tmp dir specified in mcl_tmp_dir_name */
#ifdef WIN32
	/* create the filename string first */
#ifdef	VIRTUAL_RX_MEM_SINGLE_RECEIVER_PER_HOST_MODE
	strncpy(this->vrfile_name, mcl_tmp_dir_name, MAX_FILE_NAME_LEN);
	strncat(this->vrfile_name, "mcl_vrm_tmpfile", MAX_FILE_NAME_LEN);
#else
	strncpy(this->vrfile_name, tempnam(mcl_tmp_dir_name, "mcl_vrm_tmpfile_"),
		MAX_FILE_NAME_LEN);
#endif
	/* then open the file, using in particular the LARGEFILE flag */
	if ((this->vrfile_fd = mcl_file_open(this->vrfile_name, _O_RDWR | O_CREAT | O_BINARY, 0)) < 0) {
		perror("mcl_vrmem::finish_init: ERROR");
		goto error;
	}
#else  /* UNIX case */
	/* create the filename string first */
	strncpy(this->vrfile_name, mcl_tmp_dir_name, MAX_FILE_NAME_LEN);
#ifdef	VIRTUAL_RX_MEM_SINGLE_RECEIVER_PER_HOST_MODE
	strncat(this->vrfile_name, "mcl_vrm_tmpfile", MAX_FILE_NAME_LEN);
#else
	strncat(this->vrfile_name, "mcl_vrm_tmpfile_", MAX_FILE_NAME_LEN);
	char	pid_str[10];
	memset(pid_str, '0', strlen(pid_str));
	snprintf(pid_str, 9, "%d", (INT32)getpid());
	strncat(this->vrfile_name, pid_str, MAX_FILE_NAME_LEN);
#endif
#ifdef __USE_LARGEFILE64
	/* then open the file, using in particular the LARGEFILE flag */
	if ((this->vrfile_fd = mcl_file_open(this->vrfile_name,
				O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE,
				S_IRWXU)) < 0) {
		perror("mcl_vrmem::finish_init: ERROR");
		goto error;
	}
#else /* ! __USE_LARGEFILE64 */
	if ((this->vrfile_fd = mcl_file_open(this->vrfile_name,
				O_RDWR | O_CREAT | O_TRUNC,
				S_IRWXU)) < 0) {
		perror("mcl_vrmem::finish_init: ERROR");
		goto error;
	}
#endif /* __USE_LARGEFILE64 */
#endif /* OS */
	this->initialized = 1;
	TRACELVL(5, (mcl_stdout, "<- mcl_vrmem::finish_init:\n"))
	return MCL_OK;

error:
	TRACELVL(5, (mcl_stdout, "mcl_vrmem::finish_init: ERROR\n"))
	return MCL_ERROR;
}


/**
 * Add this DU to the end of the list of DUs in _IN_VRBUF_ state.
 */
void
mcl_vrmem::add_to_end_of_vrbuf_list (class mcl_cb	*const mclcb,
				     struct du		*du)
{
	TRACELVL(5, (mcl_stdout, "   mcl_vrmem::add_to_end_of_vrbuf_list:\n"))
	if (this->in_vrbuf_list_head == NULL) {
		/* empty list */
		this->in_vrbuf_list_head = du;
		du->vrm_info.prev_in_vrbuf = du->vrm_info.next_in_vrbuf = du;
	} else {
		/* add it to the end of list */
		du_t	*tail = this->in_vrbuf_list_head->vrm_info.prev_in_vrbuf;
		du->vrm_info.prev_in_vrbuf = tail;
		tail->vrm_info.next_in_vrbuf = du;
		du->vrm_info.next_in_vrbuf = this->in_vrbuf_list_head;
		this->in_vrbuf_list_head->vrm_info.prev_in_vrbuf = du;
	}
}


/**
 * Remove this DU from the list of DUs in _IN_VRBUF_ state.
 */
void
mcl_vrmem::remove_from_vrbuf_list (class mcl_cb		*const mclcb,
				   struct du		*du)
{
	du_t	*prev = du->vrm_info.prev_in_vrbuf;
	du_t	*next = du->vrm_info.next_in_vrbuf;

	TRACELVL(5, (mcl_stdout, "   mcl_vrmem::remove_from_vrbuf_list:\n"))
	if (prev == du) {
		/* du is the only one in list */
		ASSERT(next == du);
		this->in_vrbuf_list_head = NULL;
	} else {
		prev->vrm_info.next_in_vrbuf = next;
		next->vrm_info.prev_in_vrbuf = prev;
		if (this->in_vrbuf_list_head == du) {
			/* du was the first in list */
			this->in_vrbuf_list_head = next;
		}
	}
	du->vrm_info.prev_in_vrbuf = du->vrm_info.next_in_vrbuf = NULL;
}


#endif /* VIRTUAL_RX_MEM */
