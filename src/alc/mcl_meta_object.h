/* $Id: mcl_meta_object.h,v 1.12 2005/01/11 15:40:16 moi Exp $ */
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

#ifndef MCL_METAOBJ_LAYER  /* { */
#define MCL_METAOBJ_LAYER

typedef struct object {
	UINT32		len;		/* number of bytes in this object
					 * (not padded) */
	struct object	*next;		/* next object of the same meta object
					 * in list */
	block_t		*first_blk;	/* first block of this object */
	int		first_blk_position;	/* offset of this first block (in bytes) */
	du_t		*first_du;	/* first du of this object */
	du_t		*last_decoded_du;	/* last decoded du of this object */
	int		last_decoded_du_offset;	/* offset of this last du (in bytes) */
	bool		delivered;	/* at sender side: true if the object
					 * has already been delivered to the
					 * application */
} object_t;


class mcl_meta_object{

public:

	/****** Public Members ************************************************/
	/**
	 * Constructor at a sender.
 	 * @param mclcb		(IN) mclcb.
	 */
	mcl_meta_object (class mcl_cb * mclcb);

	/**
	 * Constructor at a receiver.
	 * The object description table is passed as a parameter.
	 * @param odt		(IN) pointer to the ODT of the MO.
	 * @param odt_len	(IN) length of the ODT.
	 * @param mclcb		(IN) mclcb.
	 */
 	mcl_meta_object (char* odt, int odt_len, class mcl_cb * mclcb);

	/** Destructor */
	~mcl_meta_object ();
	
	/**
	 * Returns the meta object session id.
	 * The session identifier is set once, at session creation (at a receiver),
	 * and through set_id at a sender.
	 * @return	session id
	 */
	UINT32	get_id();

	/**
	 * Sets the meta object session id.
	 * The session identifier is set once, at session creation (at a receiver),
	 * and through set_id at a sender.
	 * @param	session id
	 */
	void	set_id(UINT32 moid);

	
	/**
	 * Adds a new object to the MO. All objects of an MO are
	 * added to the MO by using this function. Is usually called after 
	 * a mcl_meta_object (char* odt, int odt_len, class mcl_cb * mclcb). 
	 * Only at sender side.
	 * @param msg	(IN) 	Message control structure describing the object
	 * @return		<0 if error.
	 */
	int	add_object (struct mcl_msghdr	*msg);
	
	/**
	 * Creates and returns the ODT of the MO.
	 * Only at sender side.
	 * @param obj_data	(OUT) pointer to the buffer allocated for the ODT.
	 * @param obj_data	(IN-OUT) pointer to integer containing the length of the ODT.	 	 
	 */
 	void	get_object_description_table (char** odt_data, int *odt_len);
	
	/**
	 * Returns the data of the MO.
	 * Only at sender side.
	 * @param metaobj_data	(OUT) pointer to the buffer allocated for the MO.
	 * @param metaobj_len	(IN-OUT) pointer to integer containing the length of the MO.	 	 	 
	 */
 	void	get_meta_object_data (char** metaobj_data, int *metaobj_len);

	/**
	 * This function checks if objects of the MO have been decoded.
	 * It returns the length of the object and 0 if no object is available.
	 * Returns -1 if all objects have already been delivered (MO can be deleted
	 * in this case).
	 * Only at receiver side
	 * @param msg		pointer to the mcl_msghdr structure.
	 * @param flags		flags to control some specific features.
	 * @return		number of bytes received (or available if
	 * 			MCL_MSG_GET_SIZE_OF_AVAILABLE_OBJECT or MCL_PEEK is used),
 	 *			0 if no objects are ready, -1 if all objects have already 
	 *			been delivered.
	 */
 	int	check_for_decoded_objects (struct mcl_msghdr *msg, enum mcl_msgflag flags);


	/****** Public Attributes ********************************************/
	/** number of objects in this metaobject. */
	UINT32		nb_obj;

private:
	/****** Private Attributes ********************************************/
	/** id of this meta_object. */
	UINT32		moid;		
	/** pointer to head of list of objects included in the meta object. */
	struct object	*object_list_head;
	/** pointer to tail of list of objects included in the meta object. */
	struct object	*object_list_tail;
	/** number of objects delivered (sender side) in this metaobject. */
	UINT32		nb_delivered_obj;
		
	/** ptr to data buffer. */
	char		*data;			
	/** length of data buffer. */
	int 		len;
	/** pointer to the object that has been returned by MCL_MSG_GET_SIZE_OF_AVAILABLE_OBJECT
	 *  or MCL_MSG_PEEK. */
	struct object	*findobject_cache;
	
	class mcl_cb* 	mclcb;
};

#endif
