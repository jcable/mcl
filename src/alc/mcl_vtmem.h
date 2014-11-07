/* $Id: mcl_vtmem.h,v 1.3 2005/01/11 13:12:37 roca Exp $ */
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
 * "virtual transmit memory service" (vtm)
 */

#ifndef MCL_VTMEM_H  /* { */
#define MCL_VTMEM_H

#ifdef VIRTUAL_TX_MEM

/*
 * adu/vtm or du/vtm correspondance
 * stored in the adu_t and du_t structs
 */
typedef struct vtm_info {
	char		in_vtm;		/* flag: 1 if off valid (ie. in vtm), */
					/* 0 otherwise (ie. in ptm) */
	char		du_in_seq_in_txtab; /* remaining # DU in sequence in */
					/* txtab including this one */
					/* only meaningfull for a du_t struct */
	u_int		off;		/* adu or du offset within vtm file */
} vtm_info_t;


/*
 * describes each physical memory cache entry.
 * managed as a single table of vtm_cache_entry_t structs in practice.
 * the data pointer of the first entry gives the base address of the whole
 * memory block as returned by malloc.
 */
typedef struct {
	char		*data;		/* data block of this entry */
	struct du	*du;		/* owner of this entry */
} vtm_cache_entry_t;


/*
 * vtmem control block
 */
typedef struct {
	/* description of the disk file */
	/* data of all ADUs is stored in this file (name/fd) */
	char		f_name[MAX_FILE_NAME_LEN];
	int		f_fd;
	int		f_len;		/* current vtm file len */
	/* physical transmit memory area info */
	u_int		max_ptm_size;	/* max size in mem before using vtm */
	u_int		ptm_size;	/* total physical memory allocated */
	/* description of physical memory cache entries */
	u_int		cache_entry_nb;	/* actual nb of phy mem entries */
	u_int		cache_entry_size;/* actual size of each pmem entry */
	vtm_cache_entry_t *pmem_tab;	/* first phys mem entry in tab */
	/* physical memory entry management */
	u_int		next_cache_index; /* next entry to use on pmem-fault */
} vtm_cb_t;


/*
 * public variables
 */
extern u_int	vtm_cache_entry_nb;	/* total number of entries in pmem */


/*
 * usefull macros
 */
#define mcl_is_adu_in_ptm(adu)	((adu)->vtm_info.in_vtm == 0 ? 1 : 0)
#define mcl_is_du_in_ptm(du)	((du)->vtm_info.in_vtm == 0 ? 1 : 0)


#endif /* VIRTUAL_TX_MEM */

#endif /* }  MCL_VTMEM_H */
