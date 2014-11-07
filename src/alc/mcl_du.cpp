/* $Id: mcl_du.cpp,v 1.5 2005/01/11 13:12:27 roca Exp $ */
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


#include "mcl_includes.h"


/*
 * Create and init a new du_t struct.
 */
du_t*
CreateDU (mcl_cb *mclcb)
{
	du_t	*ndu;

	if (!(ndu = (du_t*)calloc(1, sizeof(du_t)))) {
		PRINT_ERR((mcl_stderr, "CreateDU: no memory"))
		mcl_exit(-1);
	}
	return (ndu);
}

/*
 * Choose one of the two methods...
 * BIN_TREE version is highly recommended with large block FEC codes,
 * SEQ is sufficient for small block FEC codes
 */
//#define BINARY_TREE_LIST_OF_DUS
#define SEQUENTIAL_LIST_OF_DUS

#ifdef BINARY_TREE_LIST_OF_DUS
TODO...
#elif defined(SEQUENTIAL_LIST_OF_DUS)

/*
 * last DU (i.e. with the highest seq nb) of the list.
 */
du_t*
LastDU (mcl_cb *mclcb,
	du_t	*list)
{
	if (list)
		return (list->prev);
	else
		return NULL;
}


/*
 * Insert the (single) du at its location (i.e. to comply
 * with the increasing seq # property) in the list.
 * WARNING: does not update the top level counter of DUs in list!
 * Returns 1 if inserted, 0 if du was duplicated.
 */
int
InsertDU (mcl_cb	*mclcb,
	  du_t		*du,		/* DU to add... */
	  du_t		**list)		/* ... here */
{
	du_t	*pdu, *ndu;	/* insert du between prev_du, next_du */

	ASSERT((du && list))
	ASSERT(!du->next && !du->prev); /* single DU to add */
	TRACELVL(5, (mcl_stdout, "-> InsertDU: du=x%x\n", (int)du))
	pdu = LastDU(mclcb, *list);	/* last DU in list, i.e. with highest seq nb */
	if (pdu == NULL) {
		/* first DU in list */
		*list = du;
		du->next = du->prev = du;
		TRACELVL(5, (mcl_stdout, "<- InsertDU: first\n"))
		return 1;
	}
	ndu = pdu->next;
	/*
	 * find the proper location of du in the list to make
	 * sure seq numbers are always increasing
	 */
	while (du->seq < pdu->seq) {
		ndu = pdu;
		pdu = pdu->prev;
		if (ndu == *list) {
			/* we have cycled ! du is the new list head */
			*list = du;
			break;
		}
	}
	if (du->seq == pdu->seq) {
		/* DU already received */
		TRACELVL(5, (mcl_stdout, "<- InsertDU: already rcvd\n"))
		return 0;
	} else {
		/* new DU */
		pdu->next = du;
		du->prev = pdu;
		du->next = ndu;
		ndu->prev = du;
		TRACELVL(5, (mcl_stdout, "<- InsertDU: new\n"))
		return 1;
	}
}

#endif /* XXX_LIST_OF_DUS */


/*
 * Each time a new FEC or data DU is received, call this function
 * to check if we received enough DUs to build the complete block.
 * Return 1 if ok, 0 otherwise.
 */
int
mcl_rx_enough_du (mcl_cb	*mclcb,
		  block_t	*blk)
{
	u_int	nb;	/* total nb of DUs received */

#ifdef FEC
	nb = blk->du_nb_in_list + blk->fec_du_nb_in_list;
#else  /* FEC */
	nb = blk->du_nb_in_list;
#endif /* FEC */
	TRACELVL(5, (mcl_stdout, "   mcl_rx_enough_du: %d expected, %d rx\n",
		blk->k, nb))
	if (nb >= blk->k)
		return 1;
	else
		return 0;
}

