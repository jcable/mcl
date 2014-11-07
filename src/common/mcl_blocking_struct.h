/* $Id: mcl_blocking_struct.h,v 1.2 2004/02/09 11:29:53 roca Exp $ */
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


/**
 * Blocking structure. Defines the number and size of each block, as
 * defined in the FLUTE document.
 */
typedef	struct mcl_blocking_struct {
	UINT32	block_nb;// number of source blocks into which the object
			// is partitioned (N parameter of the FLUTE doc).
	UINT32	I;	// number of source blocks of length A_large symbols
			// (pointer). These blocks are the first I blocks.
			// The remaining N-I blocks are of size A_small.
	UINT32	A_large;// length in source symbols of the first I large				// source blocks.
	UINT32	A_small;// length in source symbols of the remaining N-I
			// smaller blocks.
} mcl_blocking_struct_t;


/**
 * Compute source block structure.
 * Follows closely the algorithm specified in FLUTE.
 * @param B	(IN) maximum number of source symbols per source block.
 * @param L	(IN) transfer length (i.e. object size) in bytes.
 * @param E	(IN) encoding symbol length in bytes.
 * @param bs	(OUT) blocking structure, containing the N, I, A_large
 *		and A_small values. This structure must be allocated/freed
 *		by the caller.
 */
void	mcl_compute_blocking_struct (
				UINT32			B,
				UINT32			L,
				UINT32			E,
				mcl_blocking_struct_t	*bs);


