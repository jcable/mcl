/* $Id: mcl_vtmem.cpp,v 1.4 2005/01/11 13:12:37 roca Exp $ */
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
 * This file implements a "virtual memory service" where data (ie. ADUs and
 * FEC DUs) is stored on disk and copied in physical memory when required.
 * Data in physical memory is a "read-only" version of data on file.
 * We also have the possibility to store data in physical memory (ptm) untill
 * a predefined threshold is reached. Above it, data is stored in vtm.
 *
 *   +---------+       +---------+       +-----------+
 *   | on disk | store | phy mem |       | phy mem   |
 *   |  (vtm)  | <---- | cache   |       | buf (ptm) |
 *   | no size | ----> | size    |       | size      |
 *   | limit!  | page  | limit   |       | limit     |
 *   +---------+ fault +---------+       +-----------+
 *   \---------------------------/       \-----------/
 *      virtual tx mem service           keep in memory
 *
 *
 * LIMITATIONS:
 * - the disk file is shared by all mcl contexts.
 * - there is a single very large file, located by default in /tmp. Do
 *   not forget to change default location if there isn't enough room there.
 * - there is no possibility to remove a part of the file (eg. once all
 *   the DUs of an ADU have been sent. The only possibility is to remove
 *   the whole file which can only happen if all ADUs (!) have been totally
 *   sent.
 */

#include "mcl_includes.h"


#ifdef VIRTUAL_TX_MEM

/*
 * by default create that nb of phy mem.
 * NB:	there should be enough phy mem entries to store all DUs of a block!
 * 	required by the current FEC codec
 */
u_int	mcl_vtm_cache_entry_nb = 256;


/*
 * private functions
 */
static void	mcl_vtm_initialize	(mcl_cb *mclcb);
static vtm_cache_entry_t* mcl_find_empty_slot (mcl_cb *mclcb, int seq);
#ifdef NEVERDEF
static int	mcl_find_position_on_disk (mcl_cb *mclcb, du_t *du);
#else
#define mcl_find_position_on_disk(mcl_cb,du)	(du)->vtm_info.off
#endif
static int	mcl_copy_from_disk	(mcl_cb *mclcb, du_t *du, int from_fd,
					int from_off, vtm_cache_entry_t *to,
					int seq);
static vtm_cache_entry_t* mcl_find_du_in_vtm_cache (mcl_cb *mclcb, du_t *du);


/****** Public Functions ******************************************************/


/*
 * Can we use the vtm service to actually store data?
 * This is only possible if vtm service is used and there's no room
 * any more in the ptm area.
 * returns a flag: 1 if positive, 0 otherwise
 */
int
mcl_vtm_can_store_in_vtm (mcl_cb	*mclcb,
			  int		len)		/* buffer length */
{
	vtm_cb_t *vcb;		/* vtm control block ptr */

	ASSERT(mcl_cb);
	if (!mclcb->vtm_used) {
		TRACELVL(5, (mcl_stdout, "<- mcl_vtm_can_store_in_vtm: vtm not used\n"))
		return 0;
	}
	if (!mclcb->vtm_initialized)
		mcl_vtm_initialize(mcl_cb);
	vcb = &(mcl_cb->vtm_cb);
	if (vcb->ptm_size + len <= vcb->max_ptm_size) {
		TRACELVL(5, (mcl_stdout, "<- mcl_vtm_can_store_in_vtm: no, room in ptm\n"))
		return 0;
	} else {
		TRACELVL(5, (mcl_stdout, "<- mcl_vtm_can_store_in_vtm: yes\n"))
		return 1;
	}
}


/*
 * Stores data of the ADU (if not NULL) or DU (if not NULL) in the vtm
 * service.
 * Store only in file, never in the phy mem entries, as doing so would be
 * most of the time useless.
 * returns 0 if OK, < 0 if error
 */
int
mcl_vtm_store_data (mcl_cb	*mclcb,
		    adu_t	*adu,		/* NULL if not applicable */
		    du_t	*du,		/* NULL if not applicable */
		    char	*data,		/* buffer */
		    int		len,		/* buffer length */
		    int		padded_len)	/* required with adu, or 0 */
{
	u_int	off;		/* offset */
	int	l;
	char	*pad;		/* padding buffer */
	vtm_cb_t *vcb = &(mcl_cb->vtm_cb);	/* vtm control block ptr */

	ASSERT(mcl_cb);
	ASSERT(mcl_cb->vtm_used);
	TRACELVL(5, (mcl_stdout, "-> mcl_vtm_store_data:\n"))
	if (!mclcb->vtm_initialized)
		mcl_vtm_initialize(mcl_cb);
	if (adu) {
		off = adu->vtm_info.off = vcb->f_len;
		adu->vtm_info.in_vtm = 1;
		vcb->f_len += padded_len;
		adu->data = NULL;	/* not in phy mem */
	} else {
		ASSERT(du);
		off = du->vtm_info.off = vcb->f_len;
		du->vtm_info.in_vtm = 1;
		vcb->f_len += len;
		du->data = NULL;	/* not in phy mem */
	}

	/* repositionning within file is absolutely required! */
	if (lseek(vcb->f_fd, off, SEEK_SET) < 0) {
		perror("mcl_vtm_store_data: ERROR: lseek failed");
		PRINT_ERR((mcl_stderr, "mcl_vtm_store_data: ERROR, lseek (off=%u) failed\n", off));
		PRINT_ERR((mcl_stderr, "The sending file plus the FEC symbols created are probably too large!\n"))
		PRINT_ERR((mcl_stderr, "You may try to reduce the number of FEC symbols...\n"))
		goto bad;
	}
	/* store data */
	if ((l = write(vcb->f_fd, data, len)) < 0) {
		perror("mcl_vtm_store_data: ERROR: read failed");
		goto bad;
	}
	/* now store the NULL padding in the file */
	if (padded_len > len) {
		pad = (char*)calloc(padded_len - len, 1);
		if ((l = write(vcb->f_fd, pad, padded_len - len)) < 0) {
			perror("mcl_vtm_store_data: ERROR: read failed");
			goto bad;
		}
		free(pad);
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_store_data: starts at off=%d\n", off))
	return 0;

bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_store_data: ERROR\n"))
	mcl_exit(-1);
	return -1;	/* just to avoid a compiler warning! */
}


/*
 * Registers the offset in the vtm file for each du.
 * Assumes that the actual data has already been stored with the adu
 * in a previous call to mcl_vtm_store_data().
 */
void
mcl_vtm_register_du (mcl_cb	*mclcb,
		     du_t	*du,
		     u_int	off)
{
	ASSERT(mcl_cb);
	ASSERT(du);
	ASSERT(du->is_fec == false);
	/* offset within vtm file */
	du->vtm_info.off = du->block->adu->vtm_info.off + off;
	du->vtm_info.in_vtm = 1;	/* not in ptm but in vtm! */
	du->data = NULL;		/* not in phy mem */
	TRACELVL(5, (mcl_stdout,
		"   mcl_vtm_register_du: off=%d\n", du->vtm_info.off))
}


/*
 * register that this size is kept in physical memory
 */
void
mcl_vtm_register_in_ptm (mcl_cb	*mclcb,
			 adu_t		*adu,	/* NULL if not applicable */
			 du_t		*du,	/* NULL if not applicable */
			 int		len)	/* buffer length */
{
	ASSERT(mcl_cb);
	ASSERT(len > 0);
	TRACELVL(5, (mcl_stdout, "   mcl_vtm_register_in_ptm:\n"))
	mclcb->vtm_cb.ptm_size += len;
	ASSERT(mcl_cb->vtm_cb.ptm_size <= mclcb->vtm_cb.max_ptm_size);
	if (adu) {
		adu->vtm_info.in_vtm = 0;	/* false, it is in ptm */
	} else {
		ASSERT(du);
		du->vtm_info.in_vtm = 0;	/* false, it is in ptm */
	}
}


/*
 * Given a DU pointer, makes sure that the du->data pointer is a valid
 * pointer, retrieving data from disk if necessary.
 * In fact retrieves, when required, du->du_in_seq_in_txtab consecutive
 * DUs from disk.
 */
char *
mcl_vtm_get_data (mcl_cb	*mclcb,
		  du_t		*du)
{
	vtm_cache_entry_t *to;		/* put data here */
	int		from_off;	/* what offset on disk? */
	vtm_cb_t *vcb = &(mcl_cb->vtm_cb);	/* vtm control block ptr */

	ASSERT(mcl_cb);
	TRACELVL(5, (mcl_stdout, "-> mcl_vtm_get_data:\n"))
	if (du->data) {
		/* data buffer is present */
		TRACELVL(5, (mcl_stdout, "<- mcl_vtm_get_data: cached, ok\n"))
		return du->data;
	}
	ASSERT(mcl_cb->vtm_used);
	ASSERT(mcl_cb->vtm_initialized);
	ASSERT(du->vtm_info.in_vtm);
#ifdef NEVERDEF
	{
	/*
	 * check that the du_in_seq_in_txtab DUs starting from du would
	 * create a cache fault
	 */
	int	i;
	du_t	*tmp_du = du;
	for (i = du->vtm_info.du_in_seq_in_txtab; i > 0; i--) {
		ASSERT(tmp_du);
		ASSERT(tmp_du->data == NULL);
		tmp_du++;
	}
	}
#endif /* NEVERDEF */
	/* data is on disk, page fault, retrieve it first */
	if (!(to = mcl_find_empty_slot(mcl_cb, du->vtm_info.du_in_seq_in_txtab))) {
		PRINT_ERR((mcl_stderr,
		"mcl_vtm_get_data: ERROR, mcl_find_empty_slot failed\n"))
		goto bad;
	}
	if ((from_off = mcl_find_position_on_disk(mcl_cb, du)) < 0) {
		PRINT_ERR((mcl_stderr,
		"mcl_vtm_get_data: ERROR, mcl_find_position_on_disk failed\n"))
		goto bad;
	}
	if (mcl_copy_from_disk(mcl_cb, du, vcb->f_fd, from_off, to,
				du->vtm_info.du_in_seq_in_txtab)) {
		PRINT_ERR((mcl_stderr,
		"mcl_vtm_get_data: ERROR, mcl_copy_from_disk failed\n"))
		goto bad;
	}
	ASSERT(to->data == du->data);
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_get_data: from disk, ok\n"))
	return du->data;

bad:
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_get_data: ERROR\n"))
	mcl_exit(-1);
	return NULL;	/* just to avoid a compiler warning! */
}


/*
 * Frees the vtm file and closes the service.
 * returns 0 if ok, an error code otherwise
 */
int
mcl_vtm_close (mcl_cb	*mclcb)
{
	vtm_cb_t *vcb = &(mcl_cb->vtm_cb);	/* vtm control block ptr */

	if (!mclcb->vtm_initialized || !mclcb->vtm_used)
		return 0;
	TRACELVL(5, (mcl_stdout, "-> mcl_vtm_close:\n"))
	/* XXX: add a user counter and actually close when last one leaves!!! */
	mclcb->vtm_initialized = 0;
	free(vcb->pmem_tab[0].data);
	vcb->pmem_tab[0].data = NULL;	/* security */
	free(vcb->pmem_tab);
	vcb->pmem_tab = NULL;	/* security */
	close(vcb->f_fd);	/* release the fd */
	unlink(vcb->f_name);	/* remove VTMEM file */
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_close: ok\n"))
	return 0;
}


/****** Private Functions *****************************************************/


/*
 * initialize everything
 */
static void
mcl_vtm_initialize (mcl_cb	*mclcb)
{
	vtm_cache_entry_t	*pm_tab;	/* table of structs */
	char			*pm_blk;	/* actual memory block */
	vtm_cache_entry_t	*pm;		/* tmp var */
	int			i;
	u_int			off;
	vtm_cb_t		*vcb = &(mcl_cb->vtm_cb);/* vtm control block */

	TRACELVL(5, (mcl_stdout, "-> mcl_vtm_initialize:\n"))
	ASSERT(!mclcb->vtm_initialized);
	memset(vcb, 0, sizeof(vtm_cb_t));
	vcb->max_ptm_size = VIRTUAL_TX_MAX_PHY_MEM_SIZE;
	//vcb->cache_entry_size = mclcb->datagram_sz;	/* fixed size! */
	vcb->cache_entry_size = mclcb->payload_size;	/* fixed size! */
	if (!(pm_tab = (vtm_cache_entry_t*)calloc(mcl_vtm_cache_entry_nb,
						 sizeof(vtm_cache_entry_t))) ||
	    !(pm_blk = (char*)malloc(mcl_vtm_cache_entry_nb * vcb->cache_entry_size)))
	{
		PRINT_ERR((mcl_stderr,"mcl_vtmem_initialize: ERROR: out of memory"))
		mcl_exit(-1);
		return;		/* just to avoid a compiler warning! */
	}
	for (i = 0, pm = pm_tab, off = 0;
	     i < (int)mcl_vtm_cache_entry_nb;
	     i++, pm++, off += vcb->cache_entry_size) {
		pm->data = pm_blk + off;
	}
	vcb->cache_entry_nb = mcl_vtm_cache_entry_nb;
	vcb->pmem_tab = pm_tab;
	vcb->next_cache_index = 0;

	/* create temp file now in the tmp dir specified in mcl_tmp_dir_name */
#ifdef WIN32
	strncpy(vcb->f_name, tempnam(mcl_tmp_dir_name, "mcl_vtm_"),
		MAX_FILE_NAME_LEN);
	if ((vcb->f_fd = open(vcb->f_name, _O_RDWR | O_CREAT | O_BINARY)) < 0) {
		perror("mcl_vtmem_initialize: ERROR");
		PRINT_ERR((mcl_stderr,"mcl_vtmem_initialize: ERROR while creating tmp file \"%s\"", vcb->f_name))
		mcl_exit(-1);
		return;		/* just to avoid a compiler warning! */
	}
#else
	strncat(vcb->f_name, mcl_tmp_dir_name, MAX_FILE_NAME_LEN);
	strncat(vcb->f_name, "mcl_vtm_XXXXXX", MAX_FILE_NAME_LEN);
	if ((vcb->f_fd = mkstemp(vcb->f_name)) < 0) {
		perror("mcl_vtmem_initialize: ERROR");
		PRINT_ERR((mcl_stderr,"mcl_vtmem_initialize: ERROR while creating tmp file \"%s\"", vcb->f_name))
		mcl_exit(-1);
		return;		/* just to avoid a compiler warning! */
	}
#endif
	mclcb->vtm_initialized = 1;
	TRACELVL(5, (mcl_stdout, "<- mcl_vtm_initialize:\n"))
}


/*
 * find seq consecutive slots in the phy memory cache, free them all,
 * and return the first slot address.
 */
static vtm_cache_entry_t*
mcl_find_empty_slot (mcl_cb	*mclcb,
		     int	seq)	/* # consecutive empty slots to find */
{
	vtm_cache_entry_t	*pm;
	vtm_cache_entry_t	*tmp_pm;
	vtm_cb_t		*vcb = &(mcl_cb->vtm_cb);/* vtm control block */
	int			i;	/* temporary counter */

	TRACELVL(5, (mcl_stdout, "-> mcl_find_empty_slot: %d slots\n", seq))
	ASSERT(mcl_cb->vtm_initialized);
	ASSERT(vcb->next_cache_index < vcb->cache_entry_nb);
	if (vcb->next_cache_index + seq > vcb->cache_entry_nb) {
		ASSERT(seq <= (int)vcb->cache_entry_nb);
		vcb->next_cache_index = 0;
	}
	tmp_pm = pm = &(vcb->pmem_tab[vcb->next_cache_index]);
	for (i = seq; i > 0; i--, tmp_pm++) {
		ASSERT(tmp_pm);
		ASSERT(tmp_pm->data);
		if (tmp_pm->du) {
			/* mark everything (du->data and pmem entry) unused */
			tmp_pm->du->data = NULL;
			tmp_pm->du = NULL;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_find_empty_slot: index=%d\n", vcb->next_cache_index))
	/* for next time */
	vcb->next_cache_index += seq;
	vcb->next_cache_index %= vcb->cache_entry_nb;
	return(pm);
}


#ifdef NEVERDEF
/*
 * returns the offset within the vtm file, < 0 if error
 */
static int
mcl_find_position_on_disk (mcl_cb	*mclcb,
			   du_t		*du)
{
	u_int	off;	/* total offset within vtm file */

	ASSERT(mcl_cb->vtm_initialized);
	/* then add du offset within adu */
	off = du->vtm_info.off;
	TRACELVL(5, (mcl_stdout, "   mcl_find_position_on_disk: off=%d\n", off))
	return off;
}
#endif


/*
 * read a sequence of seq DUs from disk and mark that they are now in vtm cache
 */
static int
mcl_copy_from_disk (mcl_cb	*mclcb,
		    du_t	*du,
		    int		from_fd,
		    int		from_off,
		    vtm_cache_entry_t *to,
		    int		seq)	/* # consecutive empty slots to find */
{
	int			i;
	int			len;
	vtm_cb_t		*vcb = &(mcl_cb->vtm_cb); /* vtm control block */
	du_t			*tmp_du;
	vtm_cache_entry_t	*tmp_to;

	TRACELVL(5, (mcl_stdout, "-> mcl_copy_from_disk: %d DUs\n", seq))
	ASSERT(mcl_cb->vtm_initialized);
	ASSERT(seq > 0);
	/* repositionning within file is absolutely required! */
	if (lseek(from_fd, from_off, SEEK_SET) < 0) {
		PRINT_ERR((mcl_stderr, "mcl_copy_from_disk: ERROR, lseek (fd=%d, off=%d) failed\n",
			from_fd, from_off))
		perror("mcl_copy_from_disk: ERROR: lseek failed");
		mcl_exit(-1);
	}
	if ((len = read(from_fd, to->data, vcb->cache_entry_size * seq)) <= 0) {
		PRINT_ERR((mcl_stderr,"mcl_copy_from_disk: ERROR, read returned %d\n",
			len))
		perror("mcl_copy_from_disk: ERROR: read failed");
		/*
		 *PRINT_ERR((mcl_stderr,"ses_id=%d, fd=%d, data=x%x, size=%d\n",
		 *mclcb->id, from_fd, (int)to->data, vcb->cache_entry_size * seq))
		 */
		mcl_exit(-1);
	}
	for (i = seq, tmp_du = du, tmp_to = to; i > 0; i--, tmp_du++, tmp_to++) {
		ASSERT(tmp_du);
		ASSERT(tmp_to->du == NULL);
		if (tmp_du->data) {
			vtm_cache_entry_t	*cache_entry;
			/*
			 * du data can already be in cache, eg. if sent
			 * recently on another layer
			 */
			TRACE((mcl_stderr, "mcl_copy_from_disk: WARNING, du->data already in cache, index=%d\n", i))
			/*
			 * remove from previous cache entry
			 */
			cache_entry = mcl_find_du_in_vtm_cache(mcl_cb, tmp_du);
			cache_entry->du = NULL;
		}
		tmp_to->du = tmp_du;
		tmp_du->data = tmp_to->data;
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_copy_from_disk: %d copied\n", len))
	return 0;
}


/*
 * linear search in the cache table
 * not efficient but this function should be used only very occasionnaly!
 */
static vtm_cache_entry_t*
mcl_find_du_in_vtm_cache (mcl_cb	*mclcb,
			  du_t		*du)
{
	vtm_cb_t		*vcb = &(mcl_cb->vtm_cb); /* vtm control block */
	vtm_cache_entry_t	*pm;		/* tmp var */
	int			i;

	TRACELVL(5, (mcl_stdout, "-> mcl_find_du_in_vtm_cache:\n"))
	for (i = 0, pm = vcb->pmem_tab; i < (int)mcl_vtm_cache_entry_nb; i++, pm++) {
		if (pm->du == du) {
			TRACELVL(5, (mcl_stdout, "<- mcl_find_du_in_vtm_cache: found\n"))
			return pm;
		}
	}
	TRACELVL(5, (mcl_stdout, "<- mcl_find_du_in_vtm_cache: not in cache\n"))
	return NULL;
}


#endif /* VIRTUAL_TX_MEM */

