/* $Id: mcl_tx_storage.cpp,v 1.2 2004/01/30 16:27:43 roca Exp $ */
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


mcl_tx_storage::mcl_tx_storage ()
{
}

mcl_tx_storage::~mcl_tx_storage ()
{
}


mcl_error_status
mcl_tx_storage::store_adu_data (mcl_cb		*const mclcb,
				mcl_adu		*const adu,
				char		*const buf)
{
	INT32	len;		// buf actual length
	INT32	padded_len;	// buf padded length to comply with FEC req.
	char	*pbuf;		// permanent MCL internal buffer

	/*
	 * store in physical tx memory (ptm)
	 */
	ASSERT(adu);
	len = adu->get_len();
	padded_len = adu->get_padded_len();
	if (mclcb->tx.can_reuse_appli_buf()) {
		// take control of buf and avoid an extra data copy
		if (padded_len != len) {
			// add a null padding to end of block 
			if (!(pbuf = (char*)realloc((void*)buf, padded_len))) {
				goto no_mem;
			}
			memset(pbuf + len, 0, padded_len - len);/* padding */
		} else {
			pbuf = buf;	// use the appli buffer
		}
	} else {
		// copy data in a private buffer
		if (!(pbuf = (char*)malloc(padded_len))) {
			goto no_mem;
		}
		memcpy(pbuf, buf, len);
		memset(pbuf + len, 0, padded_len - len);	/* padding */
	}
	adu->set_data_ptr(pbuf);
	return MCL_OK;

no_mem:
	PRINT_ERR((mcl_stderr, "mcl_tx_storage::store_du_data: ERROR: out of memory\n"))
	return MCL_ERROR;
}


mcl_error_status
mcl_tx_storage::store_du_data  (mcl_cb		*const mclcb,
				mcl_du		*const du,
				char		*const buf)
{
	du->data = buf;
	return MCL_OK;
#if 0
	INT32	len;		// buf actual length
	char	*pbuf;		// permanent MCL internal buffer

	/*
	 * store in physical tx memory (ptm)
	 */
	ASSERT(du);
	len = du->get_len();
	if (mclcb->tx.can_reuse_appli_buf()) {
		// take control of buf and avoid an extra data copy
		// nothing to do here...
	} else {
		// copy data in a private buffer
		if (!(pbuf = (char*)malloc(len))) {
			goto no_mem;
		}
		memcpy(pbuf, buf, len);
	}
	du->set_data_ptr(pbuf);
	return MCL_OK;

no_mem:
	PRINT_ERR((mcl_stderr, "mcl_tx_storage::store_du_data: ERROR: out of memory\n"))
	return MCL_ERROR;
#endif // 0
}
