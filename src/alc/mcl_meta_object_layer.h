/* $Id: mcl_meta_object_layer.h,v 1.12 2005/01/11 15:40:16 moi Exp $ */
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

#ifndef MCL_META_OBJECT_LAYER  /* { */
#define MCL_META_OBJECT_LAYER

typedef struct list_of_meta_objects {
	class mcl_meta_object 		*meta_obj;	/* Metaobject*/
	struct list_of_meta_objects	*next;		/* next metaobject in list */
} list_of_meta_objects_t;



class mcl_meta_object_layer {
 
public:

	/****** Public Members ************************************************/
	/**
	 * Constructor.
 	 * @param mclcb		(IN) mclcb.
	 */
	mcl_meta_object_layer (class mcl_cb* mclcb);

	/** Destructor */
	~mcl_meta_object_layer ();
	
	/**
	 * Create a new meta-object at a sender.
	 * Only at sender side.
	 */
	void	create_meta_object ();
	
	/**
	 * Adds an object to the current meta-object.
	 * Is usually called after a mcl_meta_object (char* odt, int odt_len, class mcl_cb * mclcb). 
	 * Only at sender side.	 
	 * @param msg	(IN) 	Message control structure describing the object
	 * @return		<0 if error.
	 */
	int	add_object(struct mcl_msghdr	*msg);

	/**
	 * Closes the current meta object, sets its id moid and 
	 * submits it and submits the associated ODT.
	 * Only at sender side.
	 * @param		id of the current MO.
	 */
	void	close_meta_object (UINT32 moid);

	/**
	 * This function allows to pass a received ODT to the MO_mgmnt layer.
	 * It created the corresponding MO.
	 * Only at receiver side.
	 * @param		pointer to the ODT
	 * @param		length in bytes of the ODT
	 */
 	void	add_object_description_table (char* data, int len);


	/**
	 * This function checks if objects of the one of the known MO have been decoded.
	 * It returns the length of the object and 0 if no object is available.
	 * MO can be deleted that have delivered all their object are deleted by
	 * this function.
	 * Only at receiver side
	 * @param msg		pointer to the mcl_msghdr structure.
	 * @param flags		flags to control some specific features.
	 * @return		number of bytes received (or available if
	 * 			MCL_MSG_GET_SIZE_OF_AVAILABLE_OBJECT or MCL_PEEK is used),
	 *			0 if no objects are ready.
	 */
 	int	check_for_decoded_objects (struct mcl_msghdr *msg, enum mcl_msgflag	flags);
	
	/**
	 * Is id a known moid of a meta-object?
	 * Only at receiver side.
	 * @param	id of the moid
 	 * @return	true if in it is a known moid, false otherwise.
	 */
	bool	is_metaobj (UINT32 id);

	/**
	 * Returns true is the meta-object service is in use.
	 * @return	true if in use, false otherwise.
	 */
	bool	in_use();


	/**
	 * Set the meta-object service as is in use or not.
	 * @param	true = in use, false = not in use	 
	 */
	void	set_in_use(bool in_use);


	/****** Public Attributes ********************************************/
	/* TODO: make them private and use functions set/get_attribute */
	/** The currently submitted metaobject is completed (at a sender). */
	bool			completed;

	/* true if all ADU send or received objects are meta objects */
	bool 			metaobject_only_mode; 

private:
	/****** Private Attributes ********************************************/
	/** true if meta-object service in use */
	bool			used;
	
	/**
	 * At a sender, this is the currently submitted metaobject, and
	 * at a receiver, this is the last metaobject that had an object
	 * ready at last MCL_MSG_GET_SIZE_OF_AVAILABLE_OBJECT
	 * or MCL_MSG_PEEK.
	 */
	class mcl_meta_object	*current_mo;

	/** list of existing non-decoded meta objects (only at receiver). */
	struct list_of_meta_objects 	*meta_object_list;

	class mcl_cb		*mclcb;
};


//------------------------------------------------------------------------------
// Inlines for all classes follow
//------------------------------------------------------------------------------


inline bool
mcl_meta_object_layer::in_use()
{
	return this->used;
}


inline void
mcl_meta_object_layer::set_in_use(bool in_use)
{
	this->used = in_use;
}

#endif /* }  MCL_METAOBJ_LAYER_H */
