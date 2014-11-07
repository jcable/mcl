/* $Id: mcl_meta_object.cpp,v 1.28 2005/01/11 15:40:16 moi Exp $ */
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

#ifdef METAOBJECT_USED


/******************************************************************************
 * mcl_meta_object Constructor, at a sender.
 */
mcl_meta_object::mcl_meta_object (class mcl_cb* mclcb)
{
	this->moid = 0;
	this->nb_obj = 0;
	this->data = NULL;
	this->len = 0;
	this->object_list_head = NULL;
	this->object_list_tail = NULL;
	this->mclcb = mclcb;
	this->findobject_cache = NULL;
	this->nb_delivered_obj = 0;
}

/******************************************************************************
 * mcl_meta_object Constructor, at a receiver.
 */
mcl_meta_object::mcl_meta_object (char* odt, int odt_len, class mcl_cb* mclcb)
{

	UINT32 moid_length;
	UINT32 nb_obj_len;
	UINT32 obj_len_len;
	
	this->moid = 0;
	this->nb_obj = 0;
	this->data = NULL;
	this->len = 0;
	this->object_list_head = NULL;
	this->object_list_tail = NULL;
	this->mclcb = mclcb;
	this->findobject_cache = NULL;
	this->nb_delivered_obj = 0;		
	
	/* get the length of moid */
	moid_length = ((*(UINT32*)odt >> 24) & 0x0000000F);
	if (moid_length != 2) 
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object_description_table: ERROR, MOI size != 32bits not supported\n"))

	/* get the length of nb of objects */
	nb_obj_len = ((*(UINT32*)odt >> 20) & 0x0000000F);
	if (nb_obj_len != 2) 
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object_description_table: ERROR, nb_obj_len size != 32bits not supported\n"))

	/* get the length of object length */
	obj_len_len = ((*(UINT32*)odt >> 16) & 0x0000000F);
	if (obj_len_len != 4) 
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object_description_table: ERROR, obj_len_len size != 32bits not supported\n"))

	/* get the moid of the description table */
	moid = *(UINT32*)((char*)odt + 4);
	
	/* the number of objects in meta object */
	nb_obj = *(UINT32*)((char*)odt + 8);
	
	
	/* get the length of the objects out of the descritption table 
	 * and fill the object list */
	for(unsigned int i=0; i<nb_obj;i++)
	{
		if (i == 0)
		{	
			object_list_tail = object_list_head = (object*) malloc(sizeof(struct object));
		}
		else
		{
			object_list_tail->next = (object*) malloc(sizeof(struct object));
			object_list_tail = object_list_tail->next;
		}
		
		memcpy(&object_list_tail->len, (char*)odt + 12 + i*obj_len_len, obj_len_len);
		object_list_tail->delivered = false;
		object_list_tail->first_blk = NULL;
		object_list_tail->first_blk_position = 0;
		object_list_tail->last_decoded_du = NULL;
		object_list_tail->last_decoded_du_offset = 0;
		object_list_tail->first_du = NULL;		
		object_list_tail->next=NULL;

	}
	
	this->mclcb = mclcb;
}

/******************************************************************************
 * mcl_meta_object Destructor.
 */
mcl_meta_object::~mcl_meta_object ()
{
	struct object *temp;
	
	free(this->data);
	this->data = NULL;
	
	while(object_list_head!=NULL)
	{
		temp = object_list_head;
		object_list_head = object_list_head->next;
		free(temp);
	}
	object_list_tail = NULL;
	findobject_cache = NULL;
	mclcb = NULL;
}

/******************************************************************************
 * get_id : returns the MO id.
 * => See header file for more informations.
 */
UINT32	mcl_meta_object::get_id()
{
	return this->moid;
}


/******************************************************************************
 * set_id : sets the MO id.
 * => See header file for more informations.
 */
void	mcl_meta_object::set_id(UINT32 moid)
{
	this->moid = moid;
}

/******************************************************************************
 * add_object : adds an object to the MO.
 * => See header file for more informations.
 */
int mcl_meta_object::add_object(struct mcl_msghdr	*msg)
{
	struct mcl_iovec  *iov;		/* iovec */
	INT32		fd;		/* input data file (if applicable) */

	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object::add_object\n"))
	
	iov = msg->msg_iov;

	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object::add_object: function can only be used at sender side"));
	}

	if ((float) iov->iov_len > pow((float) 2,(float) ((OBJECTS_LEN_LEN*8)-1)))
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object::add_object: object size is too important (max 2^(32-1))"));
		return -1;
	}
	
	int padding_len = 0;	/* length of in bytes padding at the end of the object */
	
	/* add new object in the list of objects */
	if (object_list_tail == NULL)
	{
		object_list_tail = object_list_head = (object*) calloc(1, sizeof(struct object));
	}
	else
	{
		object_list_tail->next = (object*) calloc(1, sizeof(struct object));
		object_list_tail = object_list_tail->next;
	}

	object_list_tail->len = iov->iov_len;
	object_list_tail->delivered = false;
	object_list_tail->next=NULL;

	/* calculate padding length */
	if (iov->iov_len % mclcb->get_payload_size()!=0) padding_len = mclcb->get_payload_size() - (iov->iov_len % mclcb->get_payload_size());
	ASSERT((iov->iov_len + padding_len) % mclcb->get_payload_size() == 0)

	if (this->data == NULL)
	{
		if (!(this->data = (char*) calloc(1, this->len + iov->iov_len + padding_len))) {
			PRINT_ERR((mcl_stderr, "mcl_meta_object::add_object: Error: Cannot alloc memory!\n"))
		}
	}
	else
	{
		if (!(this->data = (char*) realloc(this->data, this->len + iov->iov_len + padding_len))) {
			PRINT_ERR((mcl_stderr, "mcl_meta_object::add_object: Error: Cannot realloc memory!\n"))
		}
	}


	/* get the object data and copy it into the metaobject buffer */
	if (iov->iov_type == MCL_IOV_TYPE_BUFFER) {
		/* copy data to meta object data */
		memcpy(this->data + this->len, iov->iov_base, iov->iov_len);
	} else {
		if (iov->iov_filename == NULL) {
			PRINT_ERR((mcl_stderr,
			"mcl_meta_object::add_object: ERROR, NULL iov_filename\n"))
			return MCL_ERROR;
		}
		fd = mcl_file_open(iov->iov_filename, O_RDONLY, 0);
		if (fd < 0) {
			PRINT_ERR((mcl_stderr,
				"mcl_meta_object::add_object: ERROR, mcl_file_open() failed for iov_filename \"%s\"\n",
				iov->iov_filename))
			return MCL_ERROR;
		}
		if (mcl_file_lseek(fd, iov->iov_offset) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_meta_object::add_object: ERROR, mcl_file_lseek() failed\n"))
			return MCL_ERROR;
		}
		if (mcl_file_read(fd, this->data, iov->iov_len) < 0) {
			PRINT_ERR((mcl_stderr,
			"mcl_meta_object::add_object: ERROR, mcl_file_read() failed\n"))
			return MCL_ERROR;
		}
	}
	
	
	/* add padding */
	memset(this->data + this->len + iov->iov_len, 0, padding_len);
	/* new meta object length */
	this->len = this->len + iov->iov_len + padding_len;
	
	nb_obj++;
	
	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::add_object\n"))

	return 0;	
}



/******************************************************************************
 * get_object_description_table : get the ODT of the MO.
 * The MODT looks like this:
 *
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  VER | ID_LEN|NBOBLEN|OBLENLEN|        PADDING	            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          MOID                                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      Number of objects                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     length of object 0                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     length of object 1                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                            .                                  |
 *  |                            .                                  |
 *  |                            .                                  | 
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |         length of object n (n = number of objects - 1)	    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * => See header file for more informations.
 */
void mcl_meta_object::get_object_description_table(char** odt_data, int *odt_len)
{
	UINT32		word1 = 0;
	struct object 	*temp;
	int 		i = 0;
	
	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object::get_object_description_table\n"))
	
	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: function can only be used at sender side"));
	}
	
	if (this->moid == 0)
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: moid has not been set"));
	}
	
	if (MOID_LEN != 2)
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: ERROR, MOI size != 32bits not supported\n"))
	if (NB_OF_OBJECTS_LEN != 2) 
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: ERROR, nb_of_objects size != 32bits not supported\n"))
	if (OBJECTS_LEN_LEN != 4) 
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: ERROR, object_size size != 32bits not supported\n"))

	/* calculate the length of the ODT that will be created */	
	*odt_len = 4 + MOID_LEN*2 + NB_OF_OBJECTS_LEN*2 + nb_obj*OBJECTS_LEN_LEN;
	
	/* allocate memory for the ODT */
	if (!(*odt_data = (char*) calloc(1, *odt_len))) {
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_object_description_table: Error: Cannot malloc memory!\n"))
	}
	
	/* create first word of the ODT */
	word1 = (ODT_VERSION << 28) | ((MOID_LEN & 0x0000000F) << 24) | ((NB_OF_OBJECTS_LEN & 0x0000000F) << 20) | ((OBJECTS_LEN_LEN & 0x0000000F) << 16);
	memcpy((void*) *odt_data, &word1, 4);
	/* copy moid */
	memcpy((void *)(((char*)(*odt_data)) + 4), &this->moid, MOID_LEN*2);	
	/* copy nb_of_objects */
	memcpy((void *)((int)*odt_data + 4 + MOID_LEN*2), &nb_obj, NB_OF_OBJECTS_LEN*2);
	/* copy all object lengths */
	temp = object_list_head;
	while(temp!=NULL)
	{
		memcpy((void *)((int)*odt_data + 4 + MOID_LEN*2 + NB_OF_OBJECTS_LEN*2 + i * OBJECTS_LEN_LEN), &temp->len, OBJECTS_LEN_LEN);
		temp = temp->next;
		i++;
	}
	
	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::get_object_description_table\n"))
	
}

/******************************************************************************
 * get_meta_object_data : get the data of the MO.
 * => See header file for more informations.
 */
void mcl_meta_object::get_meta_object_data(char** metaobj_data, int *metaobj_len)
{

	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object::get_meta_object_data\n"))
		
	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object::get_meta_object_data: function can only be used at sender side"));
	}
	
	*metaobj_data = this->data;
	*metaobj_len = this->len;

	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::get_meta_object_data\n"))
}	



/******************************************************************************
 * check_for_decoded_objects : check if objects have been decoded.
 * => See header file for more informations.
 */
int mcl_meta_object::check_for_decoded_objects(struct mcl_msghdr *msg, enum mcl_msgflag	flags)
{
	char		*userbuf;	/* user buffer */
	UINT32		userlen;	/* user buffer length */
	adu_t		*adu;		/* adu that corresponds to the meta
					   object */
	block_t		*blk;
	du_t		*du;
	UINT32		obj_position = 0;/* position (offset) of the current
					    object in the meta object */
	struct object *	temp;
	char		*dst;		/* where to do data copy in userbuf */

	/* needed for optimitzations */
	UINT32 		last_blk_position = 0;
	block_t		*last_blk = NULL;		
	
	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object::check_for_decoded_objects\n"))

	ASSERT(mclcb->is_a_receiver());

	/* search the adu that corresponds to the meta object */
	if ((adu = mclcb->rx_window.find_adu(mclcb, this->moid, -1)) == NULL)
		return 0;


	/* check if we finished delivering the MO */
	if (this->nb_delivered_obj == this->nb_obj)
	{
		/* All objects have been delivered */
		/* the adu can now be freed*/
		mcl_rx_free_this_adu(mclcb, adu);
		
		/* return code in order to free MO-instance */
		return -1;
	}

	/*
	 * then check for objects that have not been delivered before...
	 */
 	last_blk = adu->block_head;
	if (this->findobject_cache != NULL)
	{
		/* get pointer to the object that was last decoded by previous 
		 * MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT, or first
		 * object list that could not be decoded. */

		temp = object_list_head;
		while (temp != NULL && temp != this->findobject_cache)
		{
			if ((temp->len % mclcb->get_payload_size()) == 0) obj_position += temp->len;
			else obj_position += temp->len + mclcb->get_payload_size() - (temp->len % mclcb->get_payload_size());
			temp = temp->next;
		}

		this->findobject_cache = NULL;
	}
	else
		temp = object_list_head;
		
	/* go through list of objects */
	while (temp != NULL)
	{
		bool object_ready = false;

		if (!temp->delivered)
		{
			if (msg != NULL && !(flags == MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT) && msg->msg_iov->iov_len < (int) temp->len)
			{ 
				PRINT_ERR((mcl_stderr, "mcl_meta_object::check_for_decoded_objects: user buffer too small (user_buffer %i, data_size %i)!!!\n", msg->msg_iov->iov_len, temp->len))
				exit(0);
			}

			/*
			 * not already delivered: check if we can deliver it...
			 */

			/* find the right block first */
			UINT32 blk_position;
			 
			if (temp->first_blk != NULL)
			{
				blk = temp->first_blk;
				blk_position = temp->first_blk_position;
			}
			else
			{
				blk_position = last_blk_position;
				
				for (blk = last_blk; (unsigned int) blk->seq < adu->blocking_struct.block_nb; blk++)	 
				{
					if (!blk) break;
					if (blk_position + blk->len > obj_position) break;
					blk_position += blk->len;
				}
				
				temp->first_blk = blk;
				temp->first_blk_position = blk_position;
			}
			
			last_blk = blk;
			last_blk_position =  blk_position;
			
			/* we have found the right block... now check the du's */

			if (blk != NULL && blk->du_head != NULL)
			{

				UINT32 obj_len_decoded = 0;

				/* find the right du then */
				if (temp->last_decoded_du != NULL || temp->first_du != NULL)
				{
					if (temp->first_du != NULL) du = temp->first_du;
					else
					{
						obj_len_decoded = temp->last_decoded_du_offset;
						du = temp->last_decoded_du;
						blk = du->block;
					}
				}
				else
				{
				
					UINT32 i;
					/* use rint to compensate rounding errors */
					/* floor or ceil should can not be used */
					UINT32 first_du_of_obj = (UINT32) rint((double) (obj_position - blk_position) / mclcb->get_payload_size());
					for(du = blk->du_head, i = 0; i < first_du_of_obj && first_du_of_obj!=du->seq;du=du->next, i++){}
					if (first_du_of_obj==du->seq) temp->first_du = du;
					else
					{
						/* we did not received the first du of this object
						 * jump to the next object */
						goto check_next_object;
					}
				}				
				
				while((temp->first_du == du) || (du->seq == du->prev->seq + 1 && du != blk->du_head) || (du == blk->du_head && du->seq == 0))
				{
					if (!du) goto check_next_object;
					if (du->data)
					{
						obj_len_decoded += du->len;
						if (obj_len_decoded >= temp->len)
						{		
							/* object decoded !!! */
							object_ready = true;
							break; 
						}
					}
					else goto check_next_object;
					
					if (blk->du_head == du->next)
					{
						if (du->seq != blk->k - 1)
						{
							/* this is not the last du of the block */
							goto check_next_object;
						}
						/* next du is on next block */						
						blk++;
						temp->last_decoded_du_offset = obj_len_decoded;						
						temp->last_decoded_du = du;
						if ((du = blk->du_head) == NULL) goto check_next_object;
					}
					else
					{
						temp->last_decoded_du_offset = obj_len_decoded;
						temp->last_decoded_du = du;
						du=du->next;
					}
				}
						

				if (object_ready)
				{	
					/* The object is ready, and data can be delivered to appli.
					 * We now copy the data if requested by appli */
					 
					if (msg == NULL || (flags == MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT)) 
					{
						this->findobject_cache = temp;

						if (msg == NULL || flags == MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT)
						{
							/* it was just to check if an object was available... */		
							return temp->len;
						}
					}

					userbuf = (char *) msg->msg_iov->iov_base;
					userlen = msg->msg_iov->iov_len;
				
					obj_len_decoded = 0;
					dst = userbuf;
					
					blk = temp->first_blk;
					du = temp->first_du;
					
					while ((temp->first_du == du) || (du->seq == du->prev->seq + 1 && du != blk->du_head) || (du == blk->du_head && du->seq == 0))
					{
						obj_len_decoded += du->len;
						if (obj_len_decoded >= temp->len)
						{	
							/* do not copy the padding bits */
							if (obj_len_decoded > temp->len)
							{
								memcpy(dst, du->data,(int) (temp->len % mclcb->get_payload_size()));
								dst += temp->len % mclcb->get_payload_size();
								obj_len_decoded = obj_len_decoded - du->len + (temp->len % mclcb->get_payload_size());
							}
							else
							{
								memcpy(dst, du->data, (int) du->len);
								dst += du->len;
							}
							break;
						}
						else
						{
							memcpy(dst, du->data,(int) du->len);
							dst += du->len;
						}
						
						if (blk->du_head == du->next)
						{
							if (du->seq != blk->k - 1)
							{
								/* this is not the last du of the block */
								goto check_next_object;
							}
							/* next du is on next block */
							blk++;
							if ((du = blk->du_head) == NULL) goto check_next_object;
						}
						else du = du->next;
						
					}
					if (flags == MCL_MSG_DEFAULT)
					{ 
						temp->delivered = true;
						this->nb_delivered_obj++;
					}
					ASSERT(obj_len_decoded == temp->len)
					TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::check_for_decoded_objects: object of size %i returned\n", temp->len))	
					return temp->len;
				}
			}
		}
		
check_next_object:
		if (!temp->delivered && this->findobject_cache == NULL)
		{
			/* Remember first non decoded object for optimization */
			this->findobject_cache = temp;
		}

		if (mclcb->rx_window.uses_ordered_delivery_to_appli() && !temp->delivered)
		{
			/* Ordered delivery is requested, but the next object to deliver
			 * is not ready yet. */
			TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::check_for_decoded_objects: no object returned\n"))
			return 0;
		}
		
		
		/* Move to next object in list, and update obj_position, 
		 * in order to be able to locate next object */
		if ((temp->len % mclcb->get_payload_size()) == 0) obj_position += temp->len;
		else obj_position += temp->len + mclcb->get_payload_size() - (temp->len % mclcb->get_payload_size());
		temp = temp->next;
	}

	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object::check_for_decoded_objects: no object returned\n"))
	
	return 0;
}

#endif /* METAOBJECT_USED */
