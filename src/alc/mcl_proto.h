/* $Id: mcl_proto.h,v 1.21 2005/03/18 12:06:18 roca Exp $ */
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

#ifndef MCL_PROTO_H  /* { */
#define MCL_PROTO_H

/****** public function prototypes ******/


/*
 * mcl_layer.c
 */
#ifdef SVSOA_RECV
extern int	mcl_init_layer_nb	(class mcl_cb *mclcb, INT32 nb_level);
#else
extern int	mcl_init_layer_nb	(class mcl_cb *mclcb);
#endif
extern mcl_error_status mcl_init_layer_sockets (mcl_cb *mclcb);
//extern int	mcl_get_nb_fec_layers	(class mcl_cb *mclcb, INT32 k);
extern int	mcl_add_layer		(class mcl_cb *mclcb, INT32 layer);
extern int	mcl_drop_layer		(class mcl_cb *mclcb, INT32 layer, int	check);
extern int	mcl_drop_this_layer	(class mcl_cb *mclcb, INT32 layer);
extern int	mcl_free_all_layers	(class mcl_cb *const mclcb);


/*
 * mcl_sched.c
 */
extern void	UpdateTxPlanning	(class mcl_cb *mclcb, adu_t *adu_start,
					 adu_t *adu_end);
extern void	AnticipTx_UpdateTxPlanning (class mcl_cb *mclcb, adu_t *adu);


/*
 * mcl_rx.c
 */
extern void*	mcl_rx_thread		(void *arg);
extern void	mcl_process_pkt		(class mcl_cb* mclcb, mcl_rx_pkt *pkt,
					 mcl_addr *saddr, INT32 layer);
extern int	mcl_process_sig		(mcl_cb *mclcb, int type,
					 hdr_infos_t *hdr_infos,
					 mcl_addr *saddr);


/*
 * mcl_sig.c
 */
extern int	SendNONEWADU	(class mcl_cb *mclcb, INT32 max_adu);
extern int	SendCLOSE	(class mcl_cb *mclcb);
extern int	AddSigToTab	(class mcl_cb *mclcb, sig_tab_t *new_sig);
extern int	RemSigFromTab	(class mcl_cb *mclcb, INT32 level,
				 INT32 type, INT32 ext_type);
extern void	CopySigReset	(class mcl_cb *mclcb);
extern int	CanCopyMoreSig	(class mcl_cb *mclcb, INT32 level);
extern int	CopySigToLCTinfos(class mcl_cb *mclcb, INT32 level,
				hdr_infos_t *hdr_infos,
				INT32 len);
extern int	CleanupSigTab	(class mcl_cb *mclcb);
extern void	mcl_sig_close	(class mcl_cb *mclcb);


/*
 * mcl_network.c
 */
extern int	mcl_send_pkt		(class mcl_cb *mclcb,INT32 level,
					 du_t *du, adu_t *adu);
extern void	mcl_recv_pkt		(class mcl_cb *mclcb);


/*
 * mcl_du.c
 */
extern du_t*	CreateDU		(class mcl_cb *mclcb);
extern du_t*	LastDU			(class mcl_cb *mclcb, du_t *list);
extern int	InsertDU		(class mcl_cb *mclcb, du_t *du,
					 du_t **list);
extern int	mcl_rx_enough_du	(class mcl_cb *mclcb, block_t *blk);


/*
 * mcl_adu.c
 */
extern adu_t*	mcl_create_adu		(class mcl_cb *mclcb);
extern void	mcl_insert_adu		(class mcl_cb *mclcb, adu_t *adu,
					 adu_t **list);
extern adu_t*	mcl_find_adu		(class mcl_cb *mclcb, UINT32 seq,
					 INT32 FDTinstanceID, adu_t *list);
extern void	mcl_remove_adu		(class mcl_cb *mclcb, adu_t *adu,
					 adu_t	**list);
extern int	mcl_rx_new_completed_adu (class mcl_cb *mclcb, adu_t *adu);
extern int	mcl_rx_all_adu_completed (class mcl_cb *mclcb, adu_t *list);
extern block_t*	FindBlock		(class mcl_cb *mclcb, UINT32 seq,
					 block_t *list, adu_t *adu);
extern int	mcl_get_highest_toi	(mcl_cb *mclcb, adu_t *list);
extern void	mcl_tx_segment_adu	(class mcl_cb *mclcb, adu_t *adu);
extern void	mcl_tx_free_this_adu	(class mcl_cb *mclcb, adu_t *adu);
extern void	mcl_rx_free_this_adu	(class mcl_cb *mclcb, adu_t *adu);
//extern void	mcl_free_all_adu	(class mcl_cb *mclcb);


/*
 * mcl_tx_prof.c
 */
extern int		mcl_set_tx_profile	(class mcl_cb *mclcb,
						 INT32 profile);
extern mcl_error_status	mcl_print_tx_profile	(mcl_cb *mclcb);


#ifdef VIRTUAL_TX_MEM
/*
 * mcl_vtmem.c
 */
extern int	mcl_vtm_can_store_in_vtm(class mcl_cb *mclcb, INT32 len);
extern int	mcl_vtm_store_data	(class mcl_cb *mclcb, adu_t *adu,
					 du_t *du, char *data, INT32 len,
					 INT32 padded_len);
extern void	mcl_vtm_register_du	(class mcl_cb *mclcb, du_t *du,
					 UINT32 off);
extern void	mcl_vtm_register_in_ptm (class mcl_cb *mclcb, adu_t *adu,
					 du_t *du, INT32 len);
extern char*	mcl_vtm_get_data	(class mcl_cb *mclcb, du_t *du);
extern int	mcl_vtm_close		(class mcl_cb *mclcb);
#endif /* VIRTUAL_TX_MEM */


#endif /* }  MCL_PROTO_H */
