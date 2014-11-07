/* $Id: mcl_rx_window.cpp,v 1.20 2005/03/23 14:05:03 roca Exp $ */
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

#include "mcl_includes.h"


/**
 * default constructor
 */
mcl_rx_window::mcl_rx_window ()
{
	memset(this, 0, sizeof(*this));
	this->delivery_mode_to_appli = MCL_IMMEDIATE_DELIVERY;
	this->next_ordered_adu_to_return_toi = mcl_iss;
	this->nb_of_ready_adu = 0;
	this->cp2iov_fd = 0;	/* must be done here, at class init */
	
}


/**
 * default destructor
 */
mcl_rx_window::~mcl_rx_window ()
{
	/* TODO: free all remaining ADUs and DUs... */
	memset(this, 0, sizeof(*this));
}


/**
 * Set the delivery mode.
 * => See header file for more informations.
 */
mcl_error_status
mcl_rx_window::set_delivery_mode_to_appli (mcl_cb		*const mclcb,
					   mcl_delivery_mode_to_appli	mode)
{
	switch (mode) {
	case MCL_IMMEDIATE_DELIVERY:
	case MCL_ORDERED_DELIVERY:
		break;

	default:
		PRINT_ERR((mcl_stderr,
		"mcl_rx_window::set_delivery_mode_to_appli: ERROR, unsupported mode %d\n", mode))
		return MCL_ERROR;
	}
	TRACELVL(5, (mcl_stdout,
		"-> mcl_cb::set_delivery_mode_to_appli: %d", mode))
	this->delivery_mode_to_appli = mode;
	return MCL_OK;
}


/**
 * Check if all ADUs have been completed.
 * => See header file for more informations.
 */
bool
mcl_rx_window::check_if_all_adu_completed (mcl_cb	*const mclcb)
{
	adu_t		*adu;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_window::check_if_all_adu_completed:\n"))
	for (adu = this->adu_head->prev; ; adu = adu->prev) {
		if (adu->rx_status < ADU_STATUS_COMPLETED) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::check_if_all_adu_completed: no\n"))
			return false;
		}
		if (adu == this->adu_head) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::check_if_all_adu_completed: yes\n"))
			return true;
		}
	}
}


/**
 * Find the following ADU that can be returned to the receiving
 * application, while complying with the delivery mode
 * (IMMEDIATE_DELIVERY/ORDERED_DELIVERY) and FLUTE mode if applicable.
 * => See header file for more informations.
 */
adu_t *
mcl_rx_window::get_next_ready_adu (mcl_cb		*const mclcb)
{
	adu_t		*adu, *adu_to_start_with;

	TRACELVL(5, (mcl_stdout, "-> mcl_rx_window::get_next_ready_adu:\n"))
	if (this->nb_of_ready_adu <= 0) {
		/* nothing ready */
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::get_next_ready_adu: no adu available\n"))
		return NULL;
	}
	/* first check if ADU of seq == 0 is ready */
	if ((adu = this->find_adu(mclcb, 0, -1)) != NULL)
	{
		if (adu->rx_status == ADU_STATUS_DECODED) {
			TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::get_next_ready_adu: found adu seq=%d\n", adu->seq))
			return adu;
		}
	}
	if (this->uses_immediate_delivery_to_appli()) {
		/*
		 * IMMEDIATE_DELIVERY
		 */
		/*
		 * there is necessarily a ready adu.
		 * Search by increasing seq numbers, from cache
		 * or from the start...
		 */
		if (mclcb->lastadu_cache) {
			adu = mclcb->lastadu_cache;
			mclcb->lastadu_cache = NULL;
		} else {
			adu = this->adu_head;
		}
		adu_to_start_with = adu;
		do {
			if (adu->rx_status == ADU_STATUS_DECODED) {
				/*
				 * found a ready adu.
				 * it's finished, unless we are in FLUTE mode
				 * and this ADU is not desired
				 */
				if (!(mclcb->is_flute_compatible()) ||
				    mclcb->rx_flute.is_toi_requested(mclcb, adu->seq)) {
				        mclcb->lastadu_cache = adu;
					break;		/* it's ok, done */
				}
			}
			adu = adu->next;
		} while (adu != adu_to_start_with); /* while we haven't cycled*/
		if (adu->rx_status != ADU_STATUS_DECODED ||
		    (mclcb->is_flute_compatible() &&
			    !(mclcb->rx_flute.is_toi_requested(mclcb, adu->seq)))) {
			/* nothing ready */
			TRACELVL(5, (mcl_stdout,
			"<- mcl_rx_window::get_next_ready_adu: no adu available 2\n"))
			return NULL;
		}
	} else {
		/*
		 * ORDERED_DELIVERY
		 */
		ASSERT(this->uses_ordered_delivery_to_appli());
		if ((adu = this->next_ordered_adu_to_return) ||
	            (adu = this->find_adu(mclcb, this->next_ordered_adu_to_return_toi, 0))) {
			/* the next adu to return to appli is known */
			ASSERT(adu->seq == this->next_ordered_adu_to_return_toi);
			if (adu->rx_status != ADU_STATUS_DECODED) {
				this->next_ordered_adu_to_return = adu; /* remember */
				TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_window::get_next_ready_adu: next adu not ready (state=%d)\n", adu->rx_status))
				return NULL;	/* not ready */
			}
			if (mclcb->is_flute_compatible() &&
			    !(mclcb->rx_flute.is_toi_requested(mclcb, adu->seq))) {
				PRINT_ERR((mcl_stderr,
				"<- mcl_rx_window::get_next_ready_adu: ERROR, next adu not requested by appli\n"))
				return NULL;	/* not requested */
			}
		}
	}
	ASSERT(adu->rx_status == ADU_STATUS_DECODED);
	TRACELVL(5, (mcl_stdout,
	"<- mcl_rx_window::get_next_ready_adu: found adu seq=%d\n", adu->seq))
	return adu;
}


/**
 * Try to return an ADU to the appli.
 * Takes into account the various delivery modes to the application,
 * the FLUTE mode if applicable, and the application desires.
 * This function updates everything.
 * => See header file for more informations.
 */
INT32
mcl_rx_window::return_adu_to_appli (mcl_cb		*const mclcb,
				    struct mcl_msghdr	*msg,
				    enum mcl_msgflag	flags,
				    UINT32		*toi)
{
	adu_t		*adu;
	block_t		*blk;
	du_t		*du;
	INT32		i, j;
	bool		can_copy_more;	/* false once appli buffers filled */
	INT32		copied;		/* amount of data copied */
	struct mcl_iovec *iovec;	/* base of iovec vector */
	INT32		ioveclen;	/* number of entries in iovec vector */
	struct sockaddr	*saddr;
	INT32		saddr_len;

	TRACELVL(5, (mcl_stdout,
		"-> mcl_rx_window::return_adu_to_appli: msg=x%x, flags=x%x\n",
		 (INT32)msg, flags))
	/*
	 * find a ready adu first, depending on the receiving delivery mode...
	 */
	adu = mclcb->rx_window.get_next_ready_adu(mclcb);
	if (adu == NULL) {
		TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::return_adu_to_appli: no adu ready\n"))
		return -1;
	}
	ASSERT(msg != NULL);
	if (flags == MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT) {
		/* it was just to check if an ADU was available... */
		ASSERT(msg->msg_iovlen == 0);
		ASSERT(msg->msg_iov == NULL);
		msg->toi = adu->seq;
		if (toi) {
			*toi = adu->seq;
		}
#ifdef METAOBJECT_USED		
		/* check if ADU is a meta-object or if we are in meta object
		 * only mode */
		/* if we are do not return it */
		if ((mclcb->meta_obj_layer->metaobject_only_mode && adu->seq != 0) ||
		    mclcb->meta_obj_layer->is_metaobj(adu->seq)) {
			return -1;
		}
#endif
		return (adu->len);
	}
	iovec = msg->msg_iov;
	ioveclen = msg->msg_iovlen;
	if (iovec == NULL || ioveclen <= 0) {
		PRINT_ERR((mcl_stderr, "mcl_rx_window::mcl_return_data_to_appli: ERROR, bad mcl_msghdr format (NULL msg_iov or msg_iovlen <= 0)\n"))
		mcl_exit(-1);
	}

#ifdef METAOBJECT_USED
	/* check if ADU is a meta-object or if we are in meta-object only mode*/
	/* if we are do not return it */
	if ((mclcb->meta_obj_layer->metaobject_only_mode && adu->seq != 0) ||
	    mclcb->meta_obj_layer->is_metaobj(adu->seq)) {
		return -1;
	}
#endif

	/*
	 * then copy data...
	 * it requires to go through all the DUs of all the blocks of the ADU
	 * and all the iov buffers specified in the mcl_msghdr structure.
	 */
	/* initialize a context for this copy */
	if (this->init_adu_copy_to_iovec(mclcb, msg, flags) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr, "mcl_rx_window::mcl_return_data_to_appli: ERROR, init_adu_copy_to_iovec failed\n"))
		mcl_exit(-1);
	}
	can_copy_more = true;
	for (i = adu->blocking_struct.block_nb, blk = adu->block_head;
	     i > 0; i--, blk++) {
		ASSERT(blk->k <= blk->du_nb_in_list);
#ifdef FEC
		ASSERT(blk->fec_du_nb_in_list == 0);
#endif
		for (j = blk->k, du = blk->du_head;
		     j > 0 && can_copy_more;
		     j--, du = du->next) {
			/* now perform all the complex copy stuff */
			can_copy_more = this->copy_next_du_to_iovec(mclcb, du);
			if (flags == MCL_MSG_DEFAULT) {
				/* free this DU immediately to save space */
				mclcb->rx_storage.free_data(mclcb, du);
			}
		}
	}
	copied = this->get_amount_of_data_copied_to_iovec(mclcb);
	/*
	 * if in MCL_MSG_DEFAULT mode, mark the adu delivered and free its
	 * buffers (i.e. blocks, data DUs, FEC DUs)
	 */
	if (flags == MCL_MSG_DEFAULT) {
		mclcb->rx_window.mark_adu_delivered(adu);
		//adu->remove_and_free_all_buffers(mclcb);
		mcl_rx_free_this_adu(mclcb, adu);
	}
	/* else do nothing, next read will return the same data */
	/*
	 * copy source addr in application's buffer
	 */
	saddr = (struct sockaddr*)msg->msg_name;
	saddr_len = msg->msg_namelen;
	if (saddr) {
		if (adu->addr.is_ipv4_addr()) {
			if (saddr_len != (INT32)sizeof(struct sockaddr_in)) {
				PRINT_ERR((mcl_stderr,
				"mcl_rx_window::return_adu_to_appli: ERROR, addr buffer too short (%d expected for IPv4, got %d)\n",
				sizeof(struct sockaddr_in), saddr_len))
			} else {
				adu->addr.get_addr_struct((struct sockaddr_in*)saddr);
			}
		}
#ifdef INET6
		else {
			if (saddr_len != (INT32)sizeof(struct sockaddr_in6)) {
				PRINT_ERR((mcl_stderr,
				"mcl_rx_window::return_adu_to_appli: ERROR, addr buffer too short (%d expected for IPv6, got %d)\n",
				sizeof(struct sockaddr_in6), saddr_len))
			} else {
				adu->addr.get_addr_struct((struct sockaddr_in6*)saddr);
			}
		}
#endif
	}
	if (toi) {
		*toi = adu->seq;
	}
//#if 0
	if (mclcb->get_verbosity() >= 4) {
		/* dump data received, if stored in a data buffer... */
		if (msg->msg_iov->iov_type == MCL_IOV_TYPE_BUFFER &&
		    msg->msg_iov->iov_len > 0) {
			mcl_dump_buffer((char*)msg->msg_iov->iov_base,
					msg->msg_iov->iov_len,
					msg->msg_iov->iov_len >> 2);
		}
	}
//#endif
	if (mclcb->get_verbosity() == 2) {
		struct timeval	time;
		time = mcl_get_tvtime();
		PRINT_OUT((mcl_stdout, "\n%ld.%06ld\tadu_delivered seq=%d len=%d\n",
			time.tv_sec, time.tv_usec, adu->seq, adu->len))
#ifdef GET_SYSINFO
		mcl_print_sysinfo(mclcb);
#endif
	}
	/*
	 * update stats
	 */
	TRACELVL(5, (mcl_stdout, "<- mcl_rx_window::return_adu_to_appli: %d bytes returned\n", copied))
	return (copied);
}


/**
 * Update nb of ready adus plus the next adu to deliver info
 * (ORDERED_DELIVERY) and change the ADU status to delivered.
 * => See header file for more informations.
 */
void
mcl_rx_window::mark_adu_delivered (adu_t	*adu)
{
	this->nb_of_ready_adu--;
	adu->rx_status = ADU_STATUS_DELIVERED;
	if (adu->seq != 0) {
		this->next_ordered_adu_to_return_toi++;
	}
	this->next_ordered_adu_to_return = NULL; // reset to avoid problems
}


/**
 * Initialize the copy from DUs to the buffers/files specified by
 * the msghdr.
 * => See header file for more informations.
 */
mcl_error_status
mcl_rx_window::init_adu_copy_to_iovec (mcl_cb			*const mclcb,
					struct mcl_msghdr	*msg,
					enum mcl_msgflag	flags)
{
	/* reset all context variables */
	TRACELVL(5, (mcl_stdout, "   mcl_rx_window::init_adu_copy_to_iovec:\n"))
	this->cp2iov_copied = 0;
	this->cp2iov_msg = msg;
	this->cp2iov_cur_iov = msg->msg_iov;
	this->cp2iov_cur_iov_idx = 0;
	if (msg->msg_iov->iov_type == MCL_IOV_TYPE_BUFFER) {
		this->cp2iov_is_buffer = true;
		this->cp2iov_dst = (char*)this->cp2iov_cur_iov->iov_base;
	} else {
		this->cp2iov_is_buffer = false;
		if (this->cp2iov_cur_iov->iov_filename == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_rx_window::init_adu_copy_to_iovec: ERROR, NULL iov_filename\n"))
			return MCL_ERROR;
		}
		if (this->cp2iov_fd != 0) {
			/* if a previous fd is in use, close it... */
			mcl_file_close(this->cp2iov_fd);
		}
		this->cp2iov_fd = mcl_file_open(
					this->cp2iov_cur_iov->iov_filename,
#ifdef WIN32
					O_WRONLY | O_CREAT | O_BINARY | O_TRUNC,
					_S_IWRITE
#else
					O_WRONLY | O_CREAT | O_TRUNC,
					S_IRWXU
#endif
					);
		if (this->cp2iov_fd < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_rx_window::init_adu_copy_to_iovec: ERROR, mcl_file_open() failed for iov_filename \"%s\"\n",
			this->cp2iov_cur_iov->iov_filename))
			return MCL_ERROR;
		}
		if (mcl_file_lseek(this->cp2iov_fd,
			      this->cp2iov_cur_iov->iov_offset) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_rx_window::init_adu_copy_to_iovec: ERROR, mcl_file_lseek() failed\n"))
			return MCL_ERROR;
		}
	}
	this->cp2iov_rem = this->cp2iov_cur_iov->iov_len;
	return MCL_OK;
}


/**
 * Copy an individual DU to the buffers/files specified by the msghdr.
 * => See header file for more informations.
 */
bool
mcl_rx_window::copy_next_du_to_iovec (mcl_cb	*const mclcb,
				      du_t	*du)
{
	char	*from;		/* postiion in the source DU buffer */
	INT32	from_rem;	/* remaining data to copy from the DU buffer */
	INT32	len2copy;

	/* make sure data is here... */
	if (mclcb->rx_storage.get_data(mclcb, du) == MCL_ERROR) {
		PRINT_ERR((mcl_stderr,
		"mcl_fec::decode(RSE): ERROR: rx_storage.get_data() failed\n")) 
		mcl_exit(-1);
	}
	/* then copy data to iovec... */
	from = du->data;
	from_rem = du->len;
	while (1) {
		/* first copy as much as possible */
		if (this->cp2iov_rem > 0) {
			len2copy = min(from_rem, this->cp2iov_rem);
			if (this->cp2iov_is_buffer) {
				memcpy(this->cp2iov_dst, from, len2copy);
				this->cp2iov_dst += len2copy;
			} else {
				if (mcl_file_write(this->cp2iov_fd, from, len2copy) < 0) {
					PRINT_ERR((mcl_stderr,
					"mcl_rx_window::copy_next_du_to_iovec: ERROR, mcl_file_write() failed for iov %d\n", this->cp2iov_cur_iov_idx))
					mcl_exit(-1);
				}
			}
			this->cp2iov_rem -= len2copy;
			from += len2copy;
			from_rem -= len2copy;
			this->cp2iov_copied += len2copy;
			if (from_rem == 0) {
				/* nothing else to copy */
				TRACELVL(5, (mcl_stdout,
				"<- mcl_rx_window::copy_next_du_to_iovec: %d bytes from DU entirely copied\n", len2copy))
				return true;
			}
		}
		/* destination is full, so try to move to next buffer */
		this->cp2iov_cur_iov++;
		this->cp2iov_cur_iov_idx++;
		if (this->cp2iov_cur_iov_idx >= this->cp2iov_msg->msg_iovlen) {
			/* no remaining application buffer space */
			TRACELVL(5, (mcl_stdout,
			"<- mcl_rx_window::copy_next_du_to_iovec: dest buffer full\n"))
			return false;
		}
		/* can move to the following buffer */
		if (this->cp2iov_cur_iov->iov_type == MCL_IOV_TYPE_BUFFER) {
			this->cp2iov_is_buffer = true;
			this->cp2iov_dst = (char*)this->cp2iov_cur_iov->iov_base;
		} else {
			this->cp2iov_is_buffer = false;
			if (this->cp2iov_cur_iov->iov_filename == NULL) {
				PRINT_ERR((mcl_stderr,
				"mcl_rx_window::copy_next_du_to_iovec: ERROR, NULL iov_filename\n"))
				return MCL_ERROR;
			}
			if (this->cp2iov_fd != 0) {
				/* if a previous fd is in use, close it... */
				mcl_file_close(this->cp2iov_fd);
			}
			this->cp2iov_fd = mcl_file_open(
					this->cp2iov_cur_iov->iov_filename,
#ifdef WIN32
					O_WRONLY | O_CREAT | O_BINARY | O_TRUNC,
					_S_IWRITE
#else
					O_WRONLY | O_CREAT | O_TRUNC,
					S_IRWXU
#endif
					);
			if (this->cp2iov_fd < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_rx_window::copy_next_du_to_iovec: ERROR, mcl_file_open() failed for iov_filename \"%s\"\n",
				this->cp2iov_cur_iov->iov_filename))
				return MCL_ERROR;
			}
			if (mcl_file_lseek(this->cp2iov_fd,
				      this->cp2iov_cur_iov->iov_offset) < 0) {
				PRINT_ERR((mcl_stderr,
				"mcl_rx_window::copy_next_du_to_iovec: ERROR, mcl_file_lseek() failed for iov %d\n", this->cp2iov_cur_iov_idx))
				mcl_exit(-1);
			}
		}
		this->cp2iov_rem = this->cp2iov_cur_iov->iov_len;
	}
}


/**
 * Return the total amount of data copied to buffers/files since
 * the previous init_adu_copy_to_iovec.
 * => See header file for more informations.
 */
INT64
mcl_rx_window::get_amount_of_data_copied_to_iovec (mcl_cb	*const mclcb)
{
	return(this->cp2iov_copied);
}

