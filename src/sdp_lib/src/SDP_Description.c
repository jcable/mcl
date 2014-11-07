/*******************************************************************************
 * 
 * Copyright 2004 by William G. Davis.
 *
 * This library is free software released under the terms of the GNU Lesser
 * General Public License (LGPL), the full terms of which can be found in the
 * "COPYING" file that comes with the distribution.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 *------------------------------------------------------------------------------
 *
 * This file contains routines to manipulate the tree of structures defined
 * the SDP_Description.h file. See SinisterSdpDescriptions.html for
 * documentation.
 *
 ******************************************************************************/

#include <string.h>
#include "SDP_Description.h"
#include "SDP_Error.h"
#include "SDP_Str.h"
#include "SDP_LinkedList.h"
#include "SDP_Utility.h"

/*
 * We use this as a way to call SDP_CopyToStr() and ignore (well, not really)
 * its possibly error-indicating return value:
 */
#define ENSURE_COPY_TO_STR(_str_, _string_to_copy_)                         \
	do {                                                                \
		if (SDP_FAILED(SDP_CopyToStr(&_str_, _string_to_copy_)))    \
		{                                                           \
			SDP_RaiseFatalError(                                \
				SDP_ERR_OUT_OF_MEMORY,                      \
				"Couldn't allocate memory to copy string: " \
				"%s.",                                      \
				SDP_OS_ERROR_STRING                         \
			);                                                  \
			return SDP_FAILURE;                                 \
		}                                                           \
	} while (0)







/******************************************************************************
 *
 * These functions create new structures for SDP fields:
 *
 *****************************************************************************/

SDP_Description *SDP_NewDescription(void)
{
	SDP_Description *description;

	description = (SDP_Description *) SDP_Allocate(sizeof(SDP_Description));
	if (description == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for session description: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(description, 0, sizeof(SDP_Description));

	return description;
}





SDP_Owner *SDP_NewOwner(void)
{
	SDP_Owner *owner;

	owner = (SDP_Owner *) SDP_Allocate(sizeof(SDP_Owner));
	if (owner == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for session owner "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(owner, 0, sizeof(SDP_Owner));

	return owner;
}





SDP_EmailContact *SDP_NewEmailContact(void)
{
	SDP_EmailContact *email_contact;

	email_contact = (SDP_EmailContact *) SDP_Allocate(
		sizeof(SDP_EmailContact)
	);
	if (email_contact == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for email address: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(email_contact, 0, sizeof(SDP_EmailContact));

	return email_contact;
}





SDP_PhoneContact *SDP_NewPhoneContact(void)
{
	SDP_PhoneContact *phone_contact;

	phone_contact = (SDP_PhoneContact *) SDP_Allocate(
		sizeof(SDP_PhoneContact)
	);
	if (phone_contact == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for phone number: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(phone_contact, 0, sizeof(SDP_PhoneContact));

	return phone_contact;
}





SDP_Connection *SDP_NewConnection(void)
{
	SDP_Connection *connection;

	connection = (SDP_Connection *) SDP_Allocate(sizeof(SDP_Connection));
	if (connection == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for connection "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(connection, 0, sizeof(SDP_Connection));

	return connection;
}





SDP_Bandwidth *SDP_NewBandwidth(void)
{
	SDP_Bandwidth *bandwidth;

	bandwidth = (SDP_Bandwidth *) SDP_Allocate(sizeof(SDP_Bandwidth));
	if (bandwidth == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for bandwidth "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(bandwidth, 0, sizeof(SDP_Bandwidth));

	return bandwidth;
}





SDP_SessionPlayTime *SDP_NewSessionPlayTime(void)
{
	SDP_SessionPlayTime *session_play_time;

	session_play_time = (SDP_SessionPlayTime *) SDP_Allocate(
		sizeof(SDP_SessionPlayTime)
	);
	if (session_play_time == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for session start/stop time "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(session_play_time, 0, sizeof(SDP_SessionPlayTime));

	return session_play_time;
}





SDP_RepeatTime *SDP_NewRepeatTime(void)
{
	SDP_RepeatTime *repeat_time;

	repeat_time = (SDP_RepeatTime *) SDP_Allocate(sizeof(SDP_RepeatTime));
	if (repeat_time == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for new repeat time "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(repeat_time, 0, sizeof(SDP_RepeatTime));

	return repeat_time;
}





SDP_ZoneAdjustment *SDP_NewZoneAdjustment(void)
{
	SDP_ZoneAdjustment *zone_adjustment;

	zone_adjustment = (SDP_ZoneAdjustment *) SDP_Allocate(
		sizeof(SDP_ZoneAdjustment)
	);
	if (zone_adjustment == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for time zone adjustment "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(zone_adjustment, 0, sizeof(SDP_ZoneAdjustment));

	return zone_adjustment;
}





SDP_Encryption *SDP_NewEncryption(void)
{
	SDP_Encryption *encryption;

	encryption = (SDP_Encryption *) SDP_Allocate(sizeof(SDP_Encryption));
	if (encryption == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for encryption "
			"information: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(encryption, 0, sizeof(SDP_Encryption));

	return encryption;
}





SDP_Attribute *SDP_NewAttribute(void)
{
	SDP_Attribute *attribute;

	attribute = (SDP_Attribute *) SDP_Allocate(sizeof(SDP_Attribute));
	if (attribute == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for SDP attribute: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(attribute, 0, sizeof(SDP_Attribute));

	return attribute;
}





SDP_MediaDescription *SDP_NewMediaDescription(void)
{
	SDP_MediaDescription *media_description;

	media_description = (SDP_MediaDescription *) SDP_Allocate(
		sizeof(SDP_MediaDescription)
	);
	if (media_description == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory for media description: %s.",
			SDP_OS_ERROR_STRING
		);
		return NULL;
	}

	memset(media_description, 0, sizeof(SDP_MediaDescription));

	return media_description;
}







/******************************************************************************
 *
 * These functions set values within members of SDP field structures:
 *
 *****************************************************************************/

void SDP_SetProtocolVersion(
	SDP_Description *   description,
	int                 version)
{
	SDP_AssertNotNull(description);

	description->protocol_version = version;
}





int SDP_SetOwner(
	SDP_Description *   description,
	const char *        username,
	const char *        session_id,
	const char *        session_version,
	const char *        network_type,
	const char *        address_type,
	const char *        address)
{
	SDP_AssertNotNull(description);

	if (description->owner == NULL)
	{
		description->owner = SDP_NewOwner();
		if (description->owner == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(description->owner->username, username);
	ENSURE_COPY_TO_STR(description->owner->session_id, session_id);
	ENSURE_COPY_TO_STR(
		description->owner->session_version, session_version
	);
	ENSURE_COPY_TO_STR(description->owner->network_type, network_type);
	ENSURE_COPY_TO_STR(description->owner->address_type, address_type);
	ENSURE_COPY_TO_STR(description->owner->address, address);

	return SDP_SUCCESS;
}

int SDP_SetUsername(
	SDP_Owner *    owner,
	const char *   username)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->username, username);

	return SDP_SUCCESS;
}

int SDP_SetSessionID(
	SDP_Owner *    owner,
	const char *   session_id)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->session_id, session_id);

	return SDP_SUCCESS;
}

int SDP_SetSessionVersion(
	SDP_Owner *    owner,
	const char *   session_version)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->session_version, session_version);

	return SDP_SUCCESS;
}

int SDP_SetOwnerNetworkType(
	SDP_Owner *    owner,
	const char *   network_type)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->network_type, network_type);

	return SDP_SUCCESS;
}

int SDP_SetOwnerAddressType(
	SDP_Owner *    owner,
	const char *   address_type)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->address_type, address_type);

	return SDP_SUCCESS;
}

int SDP_SetOwnerAddress(
	SDP_Owner *    owner,
	const char *   address)
{
	SDP_AssertNotNull(owner);

	ENSURE_COPY_TO_STR(owner->address, address);

	return SDP_SUCCESS;
}





int SDP_SetSessionName(
	SDP_Description *   description,
	const char *        session_name)
{
	SDP_AssertNotNull(description);

	ENSURE_COPY_TO_STR(description->session_name, session_name);

	return SDP_SUCCESS;
}





int SDP_SetSessionInformation(
	SDP_Description *   description,
	const char *        session_information)
{
	SDP_AssertNotNull(description);

	ENSURE_COPY_TO_STR(
		description->session_information, session_information
	);

	return SDP_SUCCESS;
}





int SDP_SetURI(
	SDP_Description *   description,
	const char *        uri)
{
	SDP_AssertNotNull(description);

	ENSURE_COPY_TO_STR(description->uri, uri);

	return SDP_SUCCESS;
}





void SDP_AddEmailContact(
	SDP_Description *    description,
	SDP_EmailContact *   email_contact)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(email_contact);

	SDP_LINK_INTO_LIST(
		description->email_contacts,
		email_contact,
		SDP_EmailContact
	);
}

int SDP_AddNewEmailContact(
	SDP_Description *   description,
	const char *        address,
	const char *        name)
{
	SDP_EmailContact *email_contact;

	SDP_AssertNotNull(description);

	email_contact = SDP_NewEmailContact();
	if (email_contact == NULL)
		return SDP_FAILURE;

	ENSURE_COPY_TO_STR(email_contact->address, address);
	ENSURE_COPY_TO_STR(email_contact->name, name);

	SDP_AddEmailContact(description, email_contact);

	return SDP_SUCCESS;
}

int SDP_SetEmailAddress(
	SDP_EmailContact *   email_contact,
	const char *         address)
{
	SDP_AssertNotNull(email_contact);

	ENSURE_COPY_TO_STR(email_contact->address, address);

	return SDP_SUCCESS;
}

int SDP_SetEmailName(
	SDP_EmailContact *   email_contact,
	const char *         name)
{
	SDP_AssertNotNull(email_contact);

	ENSURE_COPY_TO_STR(email_contact->name, name);

	return SDP_SUCCESS;
}





void SDP_AddPhoneContact(
	SDP_Description *    description,
	SDP_PhoneContact *   phone_contact)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(phone_contact);

	SDP_LINK_INTO_LIST(
		description->phone_contacts,
		phone_contact,
		SDP_PhoneContact
	);
}

int SDP_AddNewPhoneContact(
	SDP_Description *   description,
	const char *        number,
	const char *        name)
{
	SDP_PhoneContact *phone_contact;

	SDP_AssertNotNull(description);

	phone_contact =
		(SDP_PhoneContact *) SDP_Allocate(sizeof(SDP_PhoneContact));
	if (phone_contact == NULL)
		return SDP_FAILURE;

	ENSURE_COPY_TO_STR(phone_contact->number, number);
	ENSURE_COPY_TO_STR(phone_contact->name, name);

	SDP_AddPhoneContact(description, phone_contact);

	return SDP_SUCCESS;
}

int SDP_SetPhoneNumber(
	SDP_PhoneContact *   phone_contact,
	const char *         number)
{
	SDP_AssertNotNull(phone_contact);

	ENSURE_COPY_TO_STR(phone_contact->number, number);

	return SDP_SUCCESS;
}

int SDP_SetPhoneName(
	SDP_PhoneContact *   phone_contact,
	const char *        name)
{
	SDP_AssertNotNull(phone_contact);

	ENSURE_COPY_TO_STR(phone_contact->name, name);

	return SDP_SUCCESS;
}





int SDP_SetConnection(
	SDP_Description *   description,
	const char *        network_type,
	const char *        address_type,
	const char *        address,
	int                 ttl,
	int                 total_addresses)
{
	SDP_AssertNotNull(description);

	if (description->connection == NULL)
	{
		description->connection = SDP_NewConnection();
		if (description->connection == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(
		description->connection->network_type, network_type
	);
	ENSURE_COPY_TO_STR(
		description->connection->address_type, address_type
	);
	ENSURE_COPY_TO_STR(
		description->connection->address, address
	);
	description->connection->ttl             = ttl;
	description->connection->total_addresses = total_addresses;

	return SDP_SUCCESS;
}

int SDP_SetConnectionNetworkType(
	SDP_Connection *   connection,
	const char *       network_type)
{
	SDP_AssertNotNull(connection);

	ENSURE_COPY_TO_STR(connection->network_type, network_type);

	return SDP_SUCCESS;
}

int SDP_SetConnectionAddressType(
	SDP_Connection *   connection,
	const char *       address_type)
{
	SDP_AssertNotNull(connection);

	ENSURE_COPY_TO_STR(connection->address_type, address_type);

	return SDP_SUCCESS;
}

int SDP_SetConnectionAddress(
	SDP_Connection *   connection,
	const char *       address)
{
	SDP_AssertNotNull(connection);

	ENSURE_COPY_TO_STR(connection->address, address);

	return SDP_SUCCESS;
}

void SDP_SetConnectionTTL(
	SDP_Connection *   connection,
	int                ttl)
{
	SDP_AssertNotNull(connection);

	connection->ttl = ttl;
}

void SDP_SetTotalConnectionAddresses(
	SDP_Connection *   connection,
	int                total_addresses)
{
	SDP_AssertNotNull(connection);

	connection->total_addresses = total_addresses;
}





int SDP_SetBandwidth(
	SDP_Description *   description,
	const char *        modifier,
	long                value)
{
	SDP_AssertNotNull(description);

	if (description->bandwidth == NULL)
	{
		description->bandwidth = SDP_NewBandwidth();
		if (description->bandwidth == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(description->bandwidth->modifier, modifier);
	description->bandwidth->value = value;

	return SDP_SUCCESS;
}

int SDP_SetBandwidthModifier(
	SDP_Bandwidth *   bandwidth,
	const char *      modifier)
{
	SDP_AssertNotNull(bandwidth);

	ENSURE_COPY_TO_STR(bandwidth->modifier, modifier);

	return SDP_SUCCESS;
}

void SDP_SetBandwidthValue(
	SDP_Bandwidth *   bandwidth,
	long              value)
{
	SDP_AssertNotNull(bandwidth);

	bandwidth->value = value;
}





void SDP_AddSessionPlayTime(
	SDP_Description *       description,
	SDP_SessionPlayTime *   session_play_time)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(session_play_time);

	SDP_LINK_INTO_LIST(
		description->session_play_times,
		session_play_time,
		SDP_SessionPlayTime
	);
}

int SDP_AddNewSessionPlayTime(
	SDP_Description *   description,
	time_t              start_time,
	time_t              end_time)
{
	SDP_SessionPlayTime *session_play_time;

	SDP_AssertNotNull(description);

	session_play_time = SDP_NewSessionPlayTime();
	if (session_play_time == NULL)
		return SDP_FAILURE;

	session_play_time->start_time = start_time;
	session_play_time->end_time   = end_time;

	SDP_AddSessionPlayTime(description, session_play_time);

	return SDP_SUCCESS;
}

void SDP_SetStartTime(
	SDP_SessionPlayTime *   session_play_time,
	time_t                  start_time)
{
	SDP_AssertNotNull(session_play_time);

	session_play_time->start_time = start_time;
}

void SDP_SetEndTime(
	SDP_SessionPlayTime *   session_play_time,
	time_t                  end_time)
{
	SDP_AssertNotNull(session_play_time);

	session_play_time->end_time = end_time;
}

void SDP_AddRepeatTime(
	SDP_SessionPlayTime *   session_play_time,
	SDP_RepeatTime *        repeat_time)
{
	SDP_AssertNotNull(session_play_time);
	SDP_AssertNotNull(repeat_time);

	SDP_LINK_INTO_LIST(
		session_play_time->repeat_times,
		repeat_time,
		SDP_RepeatTime
	);
}

int SDP_AddNewRepeatTime(
	SDP_SessionPlayTime *   session_play_time,
	unsigned long           repeat_interval,
	unsigned long           active_duration,
	const unsigned long     repeat_offsets[],
	int                     total_offsets)
{
	SDP_RepeatTime *repeat_time;

	SDP_AssertNotNull(session_play_time);

	repeat_time = SDP_NewRepeatTime();
	if (repeat_time == NULL)
		return SDP_FAILURE;

	repeat_time->repeat_interval = repeat_interval;
	repeat_time->active_duration = active_duration;

	if (total_offsets)
	{
		int rc = SDP_SetRepeatOffsets(
			repeat_time, repeat_offsets, total_offsets
		);
		if (SDP_FAILED(rc))
			return SDP_FAILURE;
	}

	SDP_AddRepeatTime(session_play_time, repeat_time);

	return SDP_SUCCESS;
}

void SDP_SetRepeatInterval(
	SDP_RepeatTime *   repeat_time,
	unsigned long      repeat_interval)
{
	SDP_AssertNotNull(repeat_time);

	repeat_time->repeat_interval = repeat_interval;
}

void SDP_SetActiveDuration(
	SDP_RepeatTime *   repeat_time,
	unsigned long      active_duration)
{
	SDP_AssertNotNull(repeat_time);

	repeat_time->active_duration = active_duration;
}

int SDP_SetRepeatOffsets(
	SDP_RepeatTime *      repeat_time,
	const unsigned long   repeat_offsets[],
	int                   total_offsets)
{
	SDP_AssertNotNull(repeat_time);

	if (total_offsets)
	{
		int i;

		repeat_time->repeat_offsets = (unsigned long *) SDP_Allocate(
			sizeof(unsigned long) * total_offsets
		);
		if (repeat_time->repeat_offsets == NULL)
		{
			SDP_RaiseFatalError(
				SDP_ERR_OUT_OF_MEMORY,
				"Couldn't allocate memory to copy repeat "
				"offsets: %s.",
				SDP_OS_ERROR_STRING
			);
			return SDP_FAILURE;
		}

		for (i = 0; i < total_offsets; ++i)
			repeat_time->repeat_offsets[i] = repeat_offsets[i];

		repeat_time->total_offsets = total_offsets;
	}

	return SDP_SUCCESS;
}





void SDP_AddZoneAdjustment(
	SDP_Description *      description,
	SDP_ZoneAdjustment *   zone_adjustment)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(zone_adjustment);

	SDP_LINK_INTO_LIST(
		description->zone_adjustments,
		zone_adjustment,
		SDP_ZoneAdjustment
	);
}

int SDP_AddNewZoneAdjustment(
	SDP_Description *   description,
	time_t              time,
	long                offset)
{
	SDP_ZoneAdjustment *zone_adjustment;

	SDP_AssertNotNull(description);

	zone_adjustment = SDP_NewZoneAdjustment();
	if (zone_adjustment == NULL)
		return SDP_FAILURE;

	zone_adjustment->time   = time;
	zone_adjustment->offset = offset;

	SDP_AddZoneAdjustment(description, zone_adjustment);

	return SDP_SUCCESS;
}

void SDP_SetZoneAdjustmentTime(
	SDP_ZoneAdjustment *   zone_adjustment,
	time_t                 time)
{
	SDP_AssertNotNull(zone_adjustment);

	zone_adjustment->time = time;
}

void SDP_SetZoneAdjustmentOffset(
	SDP_ZoneAdjustment *   zone_adjustment,
	long                   offset)
{
	SDP_AssertNotNull(zone_adjustment);

	zone_adjustment->offset = offset;
}





int SDP_SetEncryption(
	SDP_Description *   description,
	const char *        method,
	const char *        key)
{
	SDP_AssertNotNull(description);

	if (description->encryption == NULL)
	{
		description->encryption = SDP_NewEncryption();
		if (description->encryption == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(description->encryption->method, method);
	ENSURE_COPY_TO_STR(description->encryption->key, key);

	return SDP_SUCCESS;
}

int SDP_SetEncryptionMethod(
	SDP_Encryption *   encryption,
	const char *       method)
{
	SDP_AssertNotNull(encryption);

	ENSURE_COPY_TO_STR(encryption->method, method);

	return SDP_SUCCESS;
}

int SDP_SetEncryptionKey(
	SDP_Encryption *   encryption,
	const char *       key)
{
	SDP_AssertNotNull(encryption);

	ENSURE_COPY_TO_STR(encryption->key, key);

	return SDP_SUCCESS;
}





void SDP_AddAttribute(
	SDP_Description *   description,
	SDP_Attribute *     attribute)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(attribute);

	SDP_LINK_INTO_LIST(
		description->attributes,
		attribute,
		SDP_Attribute
	);
}

int SDP_AddNewAttribute(
	SDP_Description *   description,
	const char *        name,
	const char *        value)
{
	SDP_Attribute *attribute;

	SDP_AssertNotNull(description);

	attribute = SDP_NewAttribute();
	if (attribute == NULL)
		return SDP_FAILURE;

	ENSURE_COPY_TO_STR(attribute->name, name);
	ENSURE_COPY_TO_STR(attribute->value, value);

	SDP_AddAttribute(description, attribute);

	return SDP_SUCCESS;
}

int SDP_SetAttributeName(
	SDP_Attribute *   attribute,
	const char *      name)
{
	SDP_AssertNotNull(attribute);

	ENSURE_COPY_TO_STR(attribute->name, name);

	return SDP_SUCCESS;
}

int SDP_SetAttributeValue(
	SDP_Attribute *   attribute,
	const char *      value)
{
	SDP_AssertNotNull(attribute);

	ENSURE_COPY_TO_STR(attribute->value, value);

	return SDP_SUCCESS;
}





void SDP_AddMediaDescription(
	SDP_Description *        description,
	SDP_MediaDescription *   media_description)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(media_description);

	SDP_LINK_INTO_LIST(
		description->media_descriptions,
		media_description,
		SDP_MediaDescription
	);
}

int SDP_AddNewMediaDescription(
	SDP_Description *   description,
	const char *        media_type,
	unsigned short      port,
	unsigned short      total_ports,
	const char *        transport_protocol,
	const char *        formats,
	const char *        media_information)
{
	SDP_MediaDescription *media_description;

	SDP_AssertNotNull(description);

	media_description = SDP_NewMediaDescription();
	if (media_description == NULL)
		return SDP_FAILURE;

	SDP_AddMediaDescription(description, media_description);

	ENSURE_COPY_TO_STR(media_description->media_type, media_type);
	media_description->port        = port;
	media_description->total_ports = total_ports;
	ENSURE_COPY_TO_STR(
		media_description->transport_protocol,
		transport_protocol
	);
	ENSURE_COPY_TO_STR(media_description->formats, formats);
	ENSURE_COPY_TO_STR(
		media_description->media_information,
		media_information
	);

	return SDP_SUCCESS;
}

int SDP_SetMediaType(
	SDP_MediaDescription *   media_description,
	const char *             media_type)
{
	SDP_AssertNotNull(media_description);

	ENSURE_COPY_TO_STR(media_description->media_type, media_type);

	return SDP_SUCCESS;
}

void SDP_SetMediaPort(
	SDP_MediaDescription *   media_description,
	unsigned short           port)
{
	SDP_AssertNotNull(media_description);

	media_description->port = port;
}

void SDP_SetTotalMediaPorts(
	SDP_MediaDescription *   media_description,
	unsigned short           total_ports)
{
	SDP_AssertNotNull(media_description);

	media_description->total_ports = total_ports;
}

int SDP_SetMediaTransportProtocol(
	SDP_MediaDescription *   media_description,
	const char *             transport_protocol)
{
	SDP_AssertNotNull(media_description);

	ENSURE_COPY_TO_STR(
		media_description->transport_protocol,
		transport_protocol
	);

	return SDP_SUCCESS;
}

int SDP_SetMediaFormats(
	SDP_MediaDescription *   media_description,
	const char *             formats)
{
	SDP_AssertNotNull(media_description);

	ENSURE_COPY_TO_STR(media_description->formats, formats);

	return SDP_SUCCESS;
}

int SDP_SetMediaInformation(
	SDP_MediaDescription *   media_description,
	const char *             media_information)
{
	SDP_AssertNotNull(media_description);

	ENSURE_COPY_TO_STR(
		media_description->media_information,
		media_information
	);

	return SDP_SUCCESS;
}

int SDP_SetMediaConnection(
	SDP_MediaDescription *   media_description,
	const char *             network_type,
	const char *             address_type,
	const char *             address,
	int                      ttl,
	int                      total_addresses)
{
	SDP_AssertNotNull(media_description);

	if (media_description->connection == NULL)
	{
		media_description->connection = SDP_NewConnection();
		if (media_description->connection == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(
		media_description->connection->network_type,
		network_type
	);
	ENSURE_COPY_TO_STR(
		media_description->connection->address_type,
		address_type
	);
	ENSURE_COPY_TO_STR(
		media_description->connection->address,
		address
	);
	media_description->connection->ttl             = ttl;
	media_description->connection->total_addresses = total_addresses;

	return SDP_SUCCESS;
}

int SDP_SetMediaBandwidth(
	SDP_MediaDescription *   media_description,
	const char *             modifier,
	long                     value)
{
	SDP_AssertNotNull(media_description);

	if (media_description->bandwidth == NULL)
	{
		media_description->bandwidth = SDP_NewBandwidth();
		if (media_description->bandwidth == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(media_description->bandwidth->modifier, modifier);
	media_description->bandwidth->value = value;

	return SDP_SUCCESS;
}

int SDP_SetMediaEncryption(
	SDP_MediaDescription *   media_description,
	const char *             method,
	const char *             key)
{
	SDP_AssertNotNull(media_description);

	if (media_description->encryption == NULL)
	{
		media_description->encryption = SDP_NewEncryption();
		if (media_description->encryption == NULL)
			return SDP_FAILURE;
	}

	ENSURE_COPY_TO_STR(media_description->encryption->method, method);
	ENSURE_COPY_TO_STR(media_description->encryption->key, key);

	return SDP_SUCCESS;
}

void SDP_AddMediaAttribute(
	SDP_MediaDescription *   media_description,
	SDP_Attribute *          attribute)
{
	SDP_AssertNotNull(media_description);
	SDP_AssertNotNull(attribute);

	SDP_LINK_INTO_LIST(
		media_description->attributes,
		attribute,
		SDP_Attribute
	);
}

int SDP_AddNewMediaAttribute(
	SDP_MediaDescription *   media_description,
	const char *             name,
	const char *             value)
{
	SDP_Attribute *attribute;

	SDP_AssertNotNull(media_description);

	attribute = SDP_NewAttribute();
	if (attribute == NULL)
		return SDP_FAILURE;

	ENSURE_COPY_TO_STR(attribute->name, name);
	ENSURE_COPY_TO_STR(attribute->value, value);

	SDP_AddMediaAttribute(media_description, attribute);

	return SDP_SUCCESS;
}







/******************************************************************************
 *
 * These functions retrieve values from members within SDP field structures:
 * 
 *****************************************************************************/

int SDP_GetProtocolVersion(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return description->protocol_version;
}





SDP_Owner *SDP_GetOwner(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return description->owner;
}

const char *SDP_GetUsername(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->username) : NULL;
}

const char *SDP_GetSessionID(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->session_id) : NULL;
}

const char *SDP_GetSessionVersion(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->session_version) : NULL;
}

const char *SDP_GetOwnerNetworkType(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->network_type) : NULL;
}

const char *SDP_GetOwnerAddressType(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->address_type) : NULL;
}

const char *SDP_GetOwnerAddress(SDP_Owner *owner)
{
	return owner ? SDP_GetStrBuffer(owner->address) : NULL;
}





const char *SDP_GetSessionName(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetStrBuffer(description->session_name);
}





const char *SDP_GetSessionInformation(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetStrBuffer(description->session_information);
}





const char *SDP_GetURI(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetStrBuffer(description->uri);
}





SDP_EmailContact *SDP_GetEmailContacts(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->email_contacts);
}

const char *SDP_GetEmailAddress(SDP_EmailContact *email_contact)
{
	return email_contact ? SDP_GetStrBuffer(email_contact->address) : NULL;
}

const char *SDP_GetEmailName(SDP_EmailContact *email_contact)
{
	return email_contact ? SDP_GetStrBuffer(email_contact->name) : NULL;
}

void SDP_RemoveEmailContact(
	SDP_Description *    description,
	SDP_EmailContact *   email_contact)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(email_contact);

	SDP_UNLINK_FROM_LIST(
		description->email_contacts,
		email_contact,
		SDP_EmailContact
	);
}





SDP_PhoneContact *SDP_GetPhoneContacts(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->phone_contacts);
}

const char *SDP_GetPhoneNumber(SDP_PhoneContact *phone_contact)
{
	return phone_contact ? SDP_GetStrBuffer(phone_contact->number) : NULL;
}

const char *SDP_GetPhoneName(SDP_PhoneContact *phone_contact)
{
	return phone_contact ? SDP_GetStrBuffer(phone_contact->name) : NULL;
}

void SDP_RemovePhoneContact(
	SDP_Description *    description,
	SDP_PhoneContact *   phone_contact)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(phone_contact);

	SDP_UNLINK_FROM_LIST(
		description->phone_contacts,
		phone_contact,
		SDP_PhoneContact
	);
}





SDP_Connection *SDP_GetConnection(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return description->connection;
}

const char *SDP_GetConnectionNetworkType(SDP_Connection *connection)
{
	return connection ? SDP_GetStrBuffer(connection->network_type) : NULL;
}

const char *SDP_GetConnectionAddressType(SDP_Connection *connection)
{
	return connection ? SDP_GetStrBuffer(connection->address_type) : NULL;
}

const char *SDP_GetConnectionAddress(SDP_Connection *connection)
{
	return connection ? SDP_GetStrBuffer(connection->address) : NULL;
}

int SDP_GetConnectionTTL(SDP_Connection *connection)
{
	return connection ? connection->ttl : 0;
}

int SDP_GetTotalConnectionAddresses(SDP_Connection *connection)
{
	return connection ? connection->total_addresses : 0;
}





SDP_Bandwidth *SDP_GetBandwidth(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return description->bandwidth;
}

const char *SDP_GetBandwidthModifier(SDP_Bandwidth *bandwidth)
{
	return bandwidth ? SDP_GetStrBuffer(bandwidth->modifier) : NULL;
}

long SDP_GetBandwidthValue(SDP_Bandwidth *bandwidth)
{
	return bandwidth ? bandwidth->value : 0L;
}





SDP_SessionPlayTime *SDP_GetSessionPlayTimes(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->session_play_times);
}

time_t SDP_GetStartTime(SDP_SessionPlayTime *session_play_time)
{
	return session_play_time ? session_play_time->start_time : 0;
}

time_t SDP_GetEndTime(SDP_SessionPlayTime *session_play_time)
{
	return session_play_time ? session_play_time->end_time : 0;
}

SDP_RepeatTime *SDP_GetRepeatTimes(SDP_SessionPlayTime *session_play_time)
{
	return SDP_GetListElements(session_play_time->repeat_times);
}

unsigned long SDP_GetRepeatInterval(SDP_RepeatTime *repeat_times)
{
	return repeat_times ? repeat_times->repeat_interval : 0;
}

unsigned long SDP_GetActiveDuration(SDP_RepeatTime *repeat_times)
{
	return repeat_times ? repeat_times->active_duration : 0;
}

unsigned long *SDP_GetRepeatOffsets(SDP_RepeatTime *repeat_times)
{
	return repeat_times ? repeat_times->repeat_offsets : NULL;
}

int SDP_GetTotalRepeatOffsets(SDP_RepeatTime *repeat_times)
{
	return repeat_times ? repeat_times->total_offsets : 0;
}

void SDP_RemoveRepeatTime(
	SDP_SessionPlayTime *   session_play_times,
	SDP_RepeatTime *        repeat_time)
{
	SDP_AssertNotNull(session_play_times);
	SDP_AssertNotNull(repeat_time);

	SDP_UNLINK_FROM_LIST(
		session_play_times->repeat_times,
		repeat_time,
		SDP_SessionPlayTime
	);
}

void SDP_RemoveSessionPlayTime(
	SDP_Description *       description,
	SDP_SessionPlayTime *   session_play_time)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(session_play_time);

	SDP_UNLINK_FROM_LIST(
		description->session_play_times,
		session_play_time,
		SDP_SessionPlayTime
	);
}





SDP_ZoneAdjustment *SDP_GetZoneAdjustments(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->zone_adjustments);
}

time_t SDP_GetZoneAdjustmentTime(SDP_ZoneAdjustment *zone_adjustment)
{
	return zone_adjustment ? zone_adjustment->time : 0;
}

long SDP_GetZoneAdjustmentOffset(SDP_ZoneAdjustment *zone_adjustment)
{
	return zone_adjustment ? zone_adjustment->offset : 0;
}

void SDP_RemoveZoneAdjustment(
	SDP_Description *      description,
	SDP_ZoneAdjustment *   zone_adjustment)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(zone_adjustment);

	SDP_UNLINK_FROM_LIST(
		description->zone_adjustments,
		zone_adjustment,
		SDP_ZoneAdjustment
	);
}





SDP_Encryption *SDP_GetEncryption(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return description->encryption;
}

const char *SDP_GetEncryptionMethod(SDP_Encryption *encryption)
{
	return encryption ? SDP_GetStrBuffer(encryption->method) : NULL;
}

const char *SDP_GetEncryptionKey(SDP_Encryption *encryption)
{
	return encryption ? SDP_GetStrBuffer(encryption->key) : NULL;
}





SDP_Attribute *SDP_GetAttributes(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->attributes);
}

const char *SDP_GetAttributeName(SDP_Attribute *attribute)
{
	return attribute ? SDP_GetStrBuffer(attribute->name) : NULL;
}

const char *SDP_GetAttributeValue(SDP_Attribute *attribute)
{
	return attribute ? SDP_GetStrBuffer(attribute->value) : NULL;
}

void SDP_RemoveAttribute(
	SDP_Description *   description,
	SDP_Attribute *     attribute)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(attribute);

	SDP_UNLINK_FROM_LIST(
		description->attributes,
		attribute,
		SDP_Attribute
	);
}





SDP_MediaDescription *SDP_GetMediaDescriptions(SDP_Description *description)
{
	SDP_AssertNotNull(description);

	return SDP_GetListElements(description->media_descriptions);
}

const char *SDP_GetMediaType(SDP_MediaDescription *media_description)
{
	return media_description
		? SDP_GetStrBuffer(media_description->media_type)
		: NULL;
}

unsigned short SDP_GetMediaPort(SDP_MediaDescription *media_description)
{
	return media_description ? media_description->port : 0;
}

unsigned short SDP_GetTotalMediaPorts(SDP_MediaDescription *media_description)
{
	return media_description ? media_description->total_ports : 0;
}

const char *SDP_GetMediaTransportProtocol(
	SDP_MediaDescription *media_description)
{
	return media_description
		? SDP_GetStrBuffer(media_description->transport_protocol)
		: NULL;
}

const char *SDP_GetMediaFormats(SDP_MediaDescription *media_description)
{
	return media_description
		? SDP_GetStrBuffer(media_description->formats)
		: NULL;
}

const char *SDP_GetMediaInformation(SDP_MediaDescription *media_description)
{
	return media_description
		? SDP_GetStrBuffer(media_description->media_information)
		: NULL;
}

SDP_Connection *SDP_GetMediaConnection(SDP_MediaDescription *media_description)
{
	return media_description ? media_description->connection : NULL;
}

SDP_Bandwidth *SDP_GetMediaBandwidth(SDP_MediaDescription *media_description)
{
	return media_description ? media_description->bandwidth : NULL;
}

SDP_Encryption *SDP_GetMediaEncryption(SDP_MediaDescription *media_description)
{
	return media_description ? media_description->encryption : NULL;
}

SDP_Attribute *SDP_GetMediaAttributes(SDP_MediaDescription *media_description)
{
	return media_description
		? SDP_GetListElements(media_description->attributes)
		: NULL;
}

void SDP_RemoveMediaDescription(
	SDP_Description *        description,
	SDP_MediaDescription *   media_description)
{
	SDP_AssertNotNull(description);
	SDP_AssertNotNull(media_description);

	SDP_UNLINK_FROM_LIST(
		description->media_descriptions,
		media_description,
		SDP_MediaDescription
	);
}





/*******************************************************************************
 *
 * These functions destroy SDP structures:
 *
 ******************************************************************************/

void SDP_DestroyDescriptions(SDP_Description *descriptions)
{
	SDP_Description *description = descriptions;

	while (description)
	{
		SDP_Description *description_to_destroy = description;

		description = SDP_GetNextDescription(description);

		SDP_DestroyDescription(description_to_destroy);
	}
}

void SDP_DestroyDescription(SDP_Description *description)
{
	if (description == NULL)
		return;

	SDP_DestroyOwner(description->owner);

	SDP_DestroyStrBuffer(&description->session_name);
	SDP_DestroyStrBuffer(&description->session_information);
	SDP_DestroyStrBuffer(&description->uri);

	SDP_DestroyEmailContacts(description);
	SDP_DestroyPhoneContacts(description);
	SDP_DestroyConnection(description->connection);
	SDP_DestroyBandwidth(description->bandwidth);
	SDP_DestroySessionPlayTimes(description);
	SDP_DestroyZoneAdjustments(description);
	SDP_DestroyEncryption(description->encryption);
	SDP_DestroyAttributes(description);
	SDP_DestroyMediaDescriptions(description);

	SDP_Destroy(description);
}





void SDP_DestroyOwner(SDP_Owner *owner)
{
	if (owner == NULL)
		return;

	SDP_DestroyStrBuffer(&owner->username);
	SDP_DestroyStrBuffer(&owner->session_id);
	SDP_DestroyStrBuffer(&owner->network_type);
	SDP_DestroyStrBuffer(&owner->address_type);
	SDP_DestroyStrBuffer(&owner->address);

	SDP_Destroy(owner);
}





void SDP_DestroyEmailContacts(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->email_contacts,
		SDP_EmailContact,
		SDP_DestroyEmailContact
	);
}

void SDP_DestroyEmailContact(SDP_EmailContact *email_contact)
{
	if (email_contact == NULL)
		return;

	SDP_DestroyStrBuffer(&email_contact->address);
	SDP_DestroyStrBuffer(&email_contact->name);

	SDP_Destroy(email_contact);
}





void SDP_DestroyPhoneContacts(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->phone_contacts,
		SDP_PhoneContact,
		SDP_DestroyPhoneContact
	);
}

void SDP_DestroyPhoneContact(SDP_PhoneContact *phone_contact)
{
	if (phone_contact == NULL)
		return;

	SDP_DestroyStrBuffer(&phone_contact->number);
	SDP_DestroyStrBuffer(&phone_contact->name);

	SDP_Destroy(phone_contact);
}





void SDP_DestroyConnection(SDP_Connection *connection)
{
	if (connection == NULL)
		return;

	SDP_DestroyStrBuffer(&connection->network_type);
	SDP_DestroyStrBuffer(&connection->address_type);
	SDP_DestroyStrBuffer(&connection->address);

	SDP_Destroy(connection);
}





void SDP_DestroyBandwidth(SDP_Bandwidth *bandwidth)
{
	if (bandwidth == NULL)
		return;

	SDP_DestroyStrBuffer(&bandwidth->modifier);

	SDP_Destroy(bandwidth);
}





void SDP_DestroySessionPlayTimes(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->session_play_times,
		SDP_SessionPlayTime,
		SDP_DestroySessionPlayTime
	);
}

void SDP_DestroySessionPlayTime(SDP_SessionPlayTime *session_play_time)
{
	if (session_play_time == NULL)
		return;

	SDP_DestroyRepeatTimes(session_play_time);

	SDP_Destroy(session_play_time);
}

void SDP_DestroyRepeatTimes(SDP_SessionPlayTime *session_play_time)
{
	SDP_DESTROY_LIST(
		session_play_time->repeat_times,
		SDP_RepeatTime,
		SDP_DestroyRepeatTime
	);
}

void SDP_DestroyRepeatTime(SDP_RepeatTime *repeat_time)
{
	if (repeat_time == NULL)
		return;

	if (repeat_time->repeat_offsets)
		SDP_Destroy(repeat_time->repeat_offsets);

	SDP_Destroy(repeat_time);
}



void SDP_DestroyZoneAdjustments(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->zone_adjustments,
		SDP_ZoneAdjustment,
		SDP_DestroyZoneAdjustment
	);
}

void SDP_DestroyZoneAdjustment(SDP_ZoneAdjustment *zone_adjustment)
{
	if (zone_adjustment == NULL)
		return;

	SDP_Destroy(zone_adjustment);
}





void SDP_DestroyEncryption(SDP_Encryption *encryption)
{
	if (encryption == NULL)
		return;

	SDP_DestroyStrBuffer(&encryption->method);
	SDP_DestroyStrBuffer(&encryption->key);

	SDP_Destroy(encryption);
}





void SDP_DestroyAttributes(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->attributes,
		SDP_Attribute,
		SDP_DestroyAttribute
	);
}

void SDP_DestroyAttribute(SDP_Attribute *attribute)
{
	if (attribute == NULL)
		return;

	SDP_DestroyStrBuffer(&attribute->name);
	SDP_DestroyStrBuffer(&attribute->value);

	SDP_Destroy(attribute);
}





void SDP_DestroyMediaDescriptions(SDP_Description *description)
{
	SDP_DESTROY_LIST(
		description->media_descriptions,
		SDP_MediaDescription,
		SDP_DestroyMediaDescription
	);
}

void SDP_DestroyMediaDescription(SDP_MediaDescription *media_description)
{
	if (media_description == NULL)
		return;

	SDP_DestroyStrBuffer(&media_description->media_type);
	SDP_DestroyStrBuffer(&media_description->transport_protocol);
	SDP_DestroyStrBuffer(&media_description->formats);
	SDP_DestroyStrBuffer(&media_description->media_information);

	SDP_DestroyConnection(media_description->connection);
	SDP_DestroyBandwidth(media_description->bandwidth);
	SDP_DestroyEncryption(media_description->encryption);
	SDP_DestroyMediaAttributes(media_description);
}





void SDP_DestroyMediaAttributes(SDP_MediaDescription *media_description)
{
	SDP_DESTROY_LIST(
		media_description->attributes,
		SDP_Attribute,
		SDP_DestroyAttribute
	);
}
