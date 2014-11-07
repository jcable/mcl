/* $Id: mcl_meta_object_layer.cpp,v 1.20 2005/01/11 15:40:16 moi Exp $ */
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
 * mcl_meta_object_layer Constructor.
 */
mcl_meta_object_layer::mcl_meta_object_layer(class mcl_cb* mclcb)
{
	this->used = false;
	this->current_mo = NULL;
	this->meta_object_list = NULL;
	this->mclcb = mclcb;
	this->completed = false;
	this->metaobject_only_mode = false;
}

/******************************************************************************
 * mcl_meta_object_layer Destructor.
 */
mcl_meta_object_layer::~mcl_meta_object_layer()
{
	struct list_of_meta_objects *temp;
	
	if (mclcb->is_a_sender())
		delete current_mo;
	if (mclcb->is_a_receiver()) {
		while(meta_object_list!=NULL)
		{
			temp = meta_object_list;
			meta_object_list = meta_object_list->next;
			delete temp->meta_obj;
			free(temp);
		}
	}	
	mclcb = NULL;
	this->current_mo = NULL;
	this->used = false;

}


/******************************************************************************
 * create_meta_object : creates a new MO.
 * => See header file for more informations.
 */
void mcl_meta_object_layer::create_meta_object()
{
	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::create_meta_object: function can only be used at sender side"));
	}
	this->used = true;
	this->completed = false;
	current_mo = new mcl_meta_object(mclcb);
}


/******************************************************************************
 * add_object : adds an object to current the MO.
 * => See header file for more informations.
 */
int mcl_meta_object_layer::add_object(struct mcl_msghdr	*msg)
{
	int result;
	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object_layer::add_object\n"))
	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object: function can only be used at sender side"));
	}
	
	if (current_mo==NULL)
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object: no meta object has been declared"));
	}
	
	if ((msg->msg_iov == NULL) || (msg->msg_iovlen <= 0)) {
		PRINT_ERR((mcl_stderr,
			"mcl_meta_object_layer::add_object: ERROR: msg->msg_iov NULL or msg->msg_iovlen <= 0\n"))
		return -1;
	}

	
	result = current_mo->add_object(msg);
	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object_layer::add_object\n"))
	return result;
}

/******************************************************************************
 * add_object : closes the current MO and sets its id.
 * => See header file for more informations.
 */
void mcl_meta_object_layer::close_meta_object(UINT32 moid)
{
	char * odt;
	char * data;
	int len;
	int toi;
	int codec;
	int old_codec;
	float fec_ratio = 0;
	float old_fec_ratio;
	
	struct mcl_iovec	iov;
	struct mcl_msghdr	msg;
	
	
	TRACELVL(5, (mcl_stdout, "-> mcl_meta_object_layer::close_meta_object\n"))

	if (!mclcb->is_a_sender())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::close_meta_object: function can only be used at sender side"));
	}
		
	if (current_mo == NULL)
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::close_meta_object: no meta object has been declared"));
	}
	
	this->completed = true;

	if (current_mo->nb_obj == 0)
	{
		/* No object has been submitted between
		 * MCL_OPT_KEEP_META_OBJECT and MCL_OPT_PUSH_META_OBJECT.
		 * Just delete the MO and do nothing */
	
		delete current_mo;
		current_mo = NULL;

		return;
	}
	
	/*
	 * create a corresponding msg descriptor.
	 */
	memset((void*)&iov, 0, sizeof(iov));
	memset((void*)&msg, 0, sizeof(msg));
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_type = MCL_IOV_TYPE_BUFFER;
	
	/*save the old fec parameters*/
	old_codec = mclcb->fec.get_fec_code();
	old_fec_ratio = mclcb->fec.get_fec_ratio();
	
	/* set the moid, then get ODT and submit it to MCL */
	current_mo->set_id(moid);
	current_mo->get_object_description_table((char**)&odt, &len);
	toi = 0;
#ifdef RSE_FEC
	codec = MCL_FEC_SCHEME_RSE_129_0;
	fec_ratio = 1.5;
#else
	codec = MCL_FEC_SCHEME_NULL;
#endif

	mcl_ctl2(mclcb, MCL_OPT_SET_FEC_CODE, (void*)&codec, sizeof(codec));
	mcl_ctl2(mclcb, MCL_OPT_FEC_RATIO, (void*)&fec_ratio, sizeof(fec_ratio));
	mcl_ctl2(mclcb, MCL_OPT_SET_NEXT_TOI, &toi, sizeof(int));
	mcl_ctl2(mclcb, MCL_OPT_SET_NEXT_ADU_HIGH_IMPORTANCE, NULL, 0);
	iov.iov_base = (void*)odt;
	iov.iov_len = len;
	mcl_sendmsg2(mclcb, &msg, MCL_MSG_DEFAULT);
	
	/* get meta object data and submit it to MCL*/
	current_mo->get_meta_object_data((char**)&data, &len);
	toi = moid;
	mcl_ctl2(mclcb, MCL_OPT_SET_FEC_CODE, (void*)&old_codec, sizeof(old_codec));
	mcl_ctl2(mclcb, MCL_OPT_FEC_RATIO, (void*)&old_fec_ratio, sizeof(old_fec_ratio));	
	mcl_ctl2(mclcb, MCL_OPT_SET_NEXT_TOI, &toi, sizeof(int));
	iov.iov_base = (void*)data;
	iov.iov_len = len;
	mcl_sendmsg2(mclcb, &msg, MCL_MSG_DEFAULT);

	delete current_mo;
	current_mo = NULL;

	TRACELVL(5, (mcl_stdout, "<- mcl_meta_object_layer::close_meta_object\n"))

}

/******************************************************************************
 * add_object_description_table : allows to pass a received ODT to the MO_mgmnt layer.
 * => See header file for more informations.
 */
void mcl_meta_object_layer::add_object_description_table(char* data, int len)
{

	struct list_of_meta_objects *temp, *temp2, *new_obj;
	UINT32 moid;
	UINT32 moid_length;
	
	if (!mclcb->is_a_receiver())
	{
		PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object_description_table: function can only be used at receiver side"));
	}
	
	/* get the length of moid */
	moid_length = ((*(UINT32*)data >> 24) & 0x0000000F);
	if (moid_length != 2) PRINT_ERR((mcl_stderr, "mcl_meta_object_layer::add_object_description_table: ERROR, MOI size != 32bits not supported\n"))

	/* get the moid of the description table */
	moid = *(UINT32*)((char*)data + 4);
	
	/* check if moid is already in the list of meta objects */
	/* and create the new meta object if necessary */
	temp = temp2 = meta_object_list;
	while(temp != NULL)
	{
		if (temp->meta_obj->get_id() == moid) break;
		temp2 = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{ 
		new_obj = (list_of_meta_objects*) malloc(sizeof(struct list_of_meta_objects));
		new_obj->meta_obj = new mcl_meta_object(data, len, mclcb);
		new_obj->next = NULL;

		if ( temp2 != NULL)
		{
			temp2->next = new_obj;
		}
		else
		{
			meta_object_list = new_obj;			
		}
	}
	
}

/******************************************************************************
 * check_for_decoded_objects : Checks if objects of the one of the known MO have been decoded.
 * => See header file for more informations.
 */
int mcl_meta_object_layer::check_for_decoded_objects(struct mcl_msghdr	*msg, enum mcl_msgflag	flags)
{
	struct list_of_meta_objects *temp, *temp_prev;
	int len;

	ASSERT(mclcb->is_a_receiver());
	temp_prev = NULL;
	
	/* goto last MO used with MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT
	 * otherwise begin with first MO in list. */
	if (current_mo != NULL)
	{
		temp = meta_object_list;
		while (temp != NULL && temp->meta_obj != current_mo) 
		{
			temp_prev = temp;
			temp = temp->next;
		}
		current_mo = NULL;
	}
	else
	{
		temp = meta_object_list;
	}
	while (temp != NULL)
	{
		if ((len = temp->meta_obj->check_for_decoded_objects(msg, flags)) > 0)
		{
			/* we found an object */
			if ((flags == MCL_MSG_GET_INFO_FOR_AVAILABLE_OBJECT))
			{
				/* remember this MO in order to return the same object in
				 * next mcl_recv */
				current_mo = temp->meta_obj;
			}
			return len;
		}
		else if (len == -1) 
		{
			/* meta object can be freed*/
			
			if (temp_prev != NULL)
			{
				temp_prev->next = temp->next;
				delete temp->meta_obj;
				free(temp);
				temp = temp_prev->next;
			}
			else
			{
				meta_object_list = temp->next;	
				delete temp->meta_obj;
				free(temp);
				temp = meta_object_list;

			}
		}
		else
		{
			/* move to next MO */
			temp_prev = temp;
			temp = temp->next;
		}
	}
	return 0;
	
}

/******************************************************************************
 * check_for_decoded_objects : Is id a known moid of a meta-object?
 * => See header file for more informations.
 */
bool mcl_meta_object_layer::is_metaobj(UINT32 id)
{
	bool result = false;
	struct list_of_meta_objects* temp;
	
	temp = meta_object_list;
	while( temp != NULL)
	{
		if(id == temp->meta_obj->get_id())
		{
			result = true;
			break;
		}
		temp=temp->next;
	}

	return result;
}

#endif /* METAOBJECT_USED */
