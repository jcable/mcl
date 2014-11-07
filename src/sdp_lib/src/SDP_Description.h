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
 ******************************************************************************/

#ifndef SDP_DESCRIPTION_INCLUDED
#define SDP_DESCRIPTION_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "SDP_Str.h"
#include "SDP_LinkedList.h"


/* Skip down to SDP_Description before looking at these. */

/* The "o" field struct: */
typedef struct {
	/*
	 * A string containing the username of the originator of the session or
	 * "-" if there was none:
	 */
	SDP_Str username;

	/* A string containing the session ID: */
	SDP_Str session_id;

	/*
	 * The session version number (a string, since we don't know for sure
	 * how it will be represented):
	 */
	SDP_Str session_version;

	/*
	 * A string containing the network type portion of the "o" field
	 * (e.g., "IN" for Internet):
	 */
	SDP_Str network_type;

	/* A string containing the address type (e.g, "IP4"): */
	SDP_Str address_type;

	/* A string containing a dotted IP address (e.g, "127.0.0.1"): */
	SDP_Str address;
} SDP_Owner;



/* The "e" field struct: */
typedef struct _SDP_EmailContact {
	/* A string containing an email address: */
	SDP_Str address;

	/*
	 * A string containing the name of the supposed owner of the email
	 * address:
	 */
	SDP_Str name;

	/* A pointer to the next email address in the linked list: */
	struct _SDP_EmailContact *next;

	/* A pointer to the previous email address in the linked list: */
	struct _SDP_EmailContact *previous;
} SDP_EmailContact;



/* The "p" fields struct: */
typedef struct _SDP_PhoneContact {
	/* A string containing some kind of phone number: */
	SDP_Str number;

	/*
	 * A string containing the name of the person who can be reached at
	 * that phone number:
	 */
	SDP_Str name;

	/* A pointer to the next phone number in the linked list: */
	struct _SDP_PhoneContact *next;

	/* A pointer to the previous phone number in the linked list: */
	struct _SDP_PhoneContact *previous;
} SDP_PhoneContact;



/* The struct for session-level and media-level "c" fields: */
typedef struct {
	/*
	 * A string containing the type of network the session or media is on:
	 */
	SDP_Str network_type;

	/*
	 * A string containing address type of the address the session or media
	 * is coming from:
	 */
	SDP_Str address_type;

	/*
	 * A string containing a dotted IP address (e.g., "127.0.0.1") of the
	 * machine the session or media is coming from:
	 */
	SDP_Str address;

	/* Time To Live */
	int ttl;

	/*
	 * Counting up from the subnet in "address", how many other address are
	 * available?
	 */
	int total_addresses;
} SDP_Connection;



/* Struct for session-level and media-level "b" fields: */
typedef struct {
	/*
	 * A string containing a modifier (e.g., "CT" for conference total) of
	 * what the maximum bandwidth value applies to:
	 */
	SDP_Str modifier;

	/* A suggested maximum bandwidth value: */
	long value;
} SDP_Bandwidth;



/* The struct for the optional "r" fields that follow a "t" field: */
typedef struct _SDP_RepeatTime {
	/* This stores the repeat interval (in seconds): */
	unsigned long repeat_interval;

	/*
	 * And this stores the active duration of the session (which means
	 * the end_time field in the session play time struct below is
	 * ignored):
	 */
	unsigned long active_duration;

	/*
	 * This stores a pointer to an array of numbers, where each number is
	 * the number of seconds for each repeat offset:
	 */
	unsigned long *repeat_offsets;

	/* ...and this stores the number of elements in the array: */
	int total_offsets;

	/* A pointer to next repeat time in the linked list: */
	struct _SDP_RepeatTime *next;

	/* A pointer to the previous repeat time in the linked list: */
	struct _SDP_RepeatTime *previous;
} SDP_RepeatTime;



/* A struct for a "t" field: */
typedef struct _SDP_SessionPlayTime {
	/* The time at which the session will begin: */
	time_t start_time;

	/* The time at which the session will end: */
	time_t end_time;

	/* The linked list of repreat times for this particular time: */
	SDP_LinkedList repeat_times;

	/* The next times struct in the linked list: */
	struct _SDP_SessionPlayTime *next;

	/* The previous times struct in the linked list: */
	struct _SDP_SessionPlayTime *previous;
} SDP_SessionPlayTime;



/* The struct for "z" fields: */
typedef struct _SDP_ZoneAdjustment {
	/*
	 * At this time, there will be some sort of big shift made (e.g., time
	 * moving forward or backwards by an hour for DST)...
	 */
	time_t time;

	/* ...so adjust the repeat offests by this many seconds: */
	long offset;

	/* A pointer to the next time zone adjustment in the linked list: */
	struct _SDP_ZoneAdjustment *next;

	/* A pointer to the previous time zone adjustment in the linked list: */
	struct _SDP_ZoneAdjustment *previous;
} SDP_ZoneAdjustment;



/* The "e" field struct: */
typedef struct {
	/* A string containing the encryption method (e.g., "PGP"): */
	SDP_Str method;

	/* The encryption key: */
	SDP_Str key;
} SDP_Encryption;



/* Struct for session-level and media-level "a" fields: */
typedef struct _SDP_Attribute {
	/* A string containing some custom attribute name: */
	SDP_Str name;

	/* A string containing some custom attribute value: */
	SDP_Str value;

	/* A pointer to the next attrbute in the linked list: */
	struct _SDP_Attribute *next;

	/* A pointer to the previous attrbute in the linked list: */
	struct _SDP_Attribute *previous;
} SDP_Attribute;



/* Struct for media description "m" fields: */
typedef struct _SDP_MediaDescription {
	/* A string containing the media type (e.g., "audio"): */
	SDP_Str media_type;

	/* The port number of where the media will be sent: */
	unsigned short port;

	/* Counting up from port, how many other ports should be used? */
	unsigned short total_ports;

	/*
	 * A string containing the name of the protocol to be used to deliver
	 * the stream (e.g., "RTP/AVP"):
	 */
	SDP_Str transport_protocol;

	/* A string containing the media formats: */
	SDP_Str formats;

	/* A string containing a description of the media: */
	SDP_Str media_information;

	/*
	 * This will store connection information for this specific piece of
	 * media if there was no session-level "c" field or if the connection
	 * used for this media differs from that used by the rest of the
	 * session:
	 */
	SDP_Connection *connection;

	/*
	 * This will store a suggested maximum bandwidth limit for this media
	 * if there was no session-level "b" field or if the bandwidth limits
	 * differ for this media differ than those for the entire session
	 */
	SDP_Bandwidth *bandwidth;

	/*
	 * Encryption information for the media, if there was none for the
	 * session or if it differs from the session's.
	 */
	SDP_Encryption *encryption;

	/*
	 * A linked list of attribute structs containing the media-level
	 * attributes for this piece of media:
	 */
	SDP_LinkedList attributes;

	/* A pointer to the next media description in the linked list: */
	struct _SDP_MediaDescription *next;

	/* A pointer to the previous media description in the linked list: */
	struct _SDP_MediaDescription *previous;
} SDP_MediaDescription;



/*
 * The SDP_Description structure. This contains pointers to all of the other
 * structures above:
 */
typedef struct _SDP_Description {
	/* The protocol version number from the "v" field: */
	int protocol_version;

	/* A struct contanining data extracted from the "o" field: */
	SDP_Owner *owner;

	/* A string containing the session name from the "s" field: */
	SDP_Str session_name;

	/* A string containing the session information from the "i" field: */
	SDP_Str session_information;

	/* A string containing the session URI from the "u" field: */
	SDP_Str uri;

	/* A linked list containing each parsed email address ("e") field: */
	SDP_LinkedList email_contacts;

	/* A linked list containing each parsed phone number ("p") field: */
	SDP_LinkedList phone_contacts;

	/* Pointer to session-level connection information from a "c" field: */
	SDP_Connection *connection;

	/*
	 * Pointer to session-level suggested maximum bandwidth limits from a
	 * "b" field:
	 */ 
	SDP_Bandwidth *bandwidth;

	/*
	 * A pointer to a linked list containing parsed "t" fields and the "r"
	 * fields that optionally follow each one:
	 */
	SDP_LinkedList session_play_times;

	/*
	 * A pointer to a linked list containing each parsed timezone
	 * adjustment for any repeat offsets:
	 */
	SDP_LinkedList zone_adjustments;

	/* The session encryption method and key: */
	SDP_Encryption *encryption;
	
	/* A linked list containing the session-level attributes: */
	SDP_LinkedList attributes;

	/* A linked list containing the description of each media: */
	SDP_LinkedList media_descriptions;

	/*
	 * If there are multiple session descriptions within a single input
	 * stream, then this will store a pointer to the next one in the
	 * linked list...
	 */
	struct _SDP_Description *next;

	/* ...and this will store a pointer to the previous one: */
	struct _SDP_Description *previous;
} SDP_Description;



extern SDP_Description *SDP_NewDescription(void);
extern SDP_Owner *SDP_NewOwner(void);
extern SDP_EmailContact *SDP_NewEmailContact(void);
extern SDP_PhoneContact *SDP_NewPhoneContact(void);
extern SDP_Connection *SDP_NewConnection(void);
extern SDP_Bandwidth *SDP_NewBandwidth(void);
extern SDP_SessionPlayTime *SDP_NewSessionPlayTime(void);
extern SDP_RepeatTime *SDP_NewRepeatTime(void);
extern SDP_ZoneAdjustment *SDP_NewZoneAdjustment(void);
extern SDP_Encryption *SDP_NewEncryption(void);
extern SDP_Attribute *SDP_NewAttribute(void);
extern SDP_MediaDescription *SDP_NewMediaDescription(void);

extern void SDP_SetProtocolVersion(
	SDP_Description *   description,
	int                 version
);
extern int SDP_SetOwner(
	SDP_Description *   description,
	const char *        username,
	const char *        session_id,
	const char *        session_version,
	const char *        network_type,
	const char *        address_type,
	const char *        address
);
extern int SDP_SetUsername(
	SDP_Owner *    owner,
	const char *   username
);
extern int SDP_SetSessionID(
	SDP_Owner *    owner,
	const char *   session_id
);
extern int SDP_SetSessionVersion(
	SDP_Owner *    owner,
	const char *   session_version
);
extern int SDP_SetOwnerNetworkType(
	SDP_Owner *    owner,
	const char *   network_type
);
extern int SDP_SetOwnerAddressType(
	SDP_Owner *    owner,
	const char *   address_type
);
extern int SDP_SetOwnerAddress(
	SDP_Owner *    owner,
	const char *   address
);
extern int SDP_SetSessionName(
	SDP_Description *   description,
	const char *        session_name
);
extern int SDP_SetSessionInformation(
	SDP_Description *   description,
	const char *        session_information
);
extern int SDP_SetURI(
	SDP_Description *   description,
	const char *        uri
);
extern void SDP_AddEmailContact(
	SDP_Description *    description,
	SDP_EmailContact *   email_contact
);
extern int SDP_AddNewEmailContact(
	SDP_Description *   description,
	const char *        address,
	const char *        name
);
extern int SDP_SetEmailAddress(
	SDP_EmailContact *   email_contact,
	const char *         address
);
extern int SDP_SetEmailName(
	SDP_EmailContact *   email_contact,
	const char *         name
);
extern void SDP_AddPhoneContact(
	SDP_Description *    description,
	SDP_PhoneContact *   phone_contact
);
extern int SDP_AddNewPhoneContact(
	SDP_Description *   description,
	const char *        number,
	const char *        name
);
extern int SDP_SetPhoneNumber(
	SDP_PhoneContact *   phone_contact,
	const char *         number
);
extern int SDP_SetPhoneName(
	SDP_PhoneContact *   phone_contact,
	const char *        name
);
extern int SDP_SetConnection(
	SDP_Description *   description,
	const char *        network_type,
	const char *        address_type,
	const char *        address,
	int                 ttl,
	int                 total_addresses
);
extern int SDP_SetConnectionNetworkType(
	SDP_Connection *   connection,
	const char *       network_type
);
extern int SDP_SetConnectionAddressType(
	SDP_Connection *   connection,
	const char *       address_type
);
extern int SDP_SetConnectionAddress(
	SDP_Connection *   connection,
	const char *       address
);
extern void SDP_SetConnectionTTL(
	SDP_Connection *   connection,
	int                ttl
);
extern void SDP_SetTotalConnectionAddresses(
	SDP_Connection *   connection,
	int                total_addresses
);
extern int SDP_SetBandwidth(
	SDP_Description *   description,
	const char *        modifier,
	long                value
);
extern int SDP_SetBandwidthModifier(
	SDP_Bandwidth *   bandwidth,
	const char *      modifier
);
extern void SDP_SetBandwidthValue(
	SDP_Bandwidth *   bandwidth,
	long              value
);
extern void SDP_AddSessionPlayTime(
	SDP_Description *       description,
	SDP_SessionPlayTime *   session_play_time
);
extern int SDP_AddNewSessionPlayTime(
	SDP_Description *   description,
	time_t              start_time,
	time_t              end_time
);
extern void SDP_SetStartTime(
	SDP_SessionPlayTime *   session_play_time,
	time_t                  start_time
);
extern void SDP_SetEndTime(
	SDP_SessionPlayTime *   session_play_time,
	time_t                  end_time
);
extern void SDP_AddRepeatTime(
	SDP_SessionPlayTime *   session_play_time,
	SDP_RepeatTime *        repeat_time
);
extern int SDP_AddNewRepeatTime(
	SDP_SessionPlayTime *   session_play_time,
	unsigned long           repeat_interval,
	unsigned long           active_duration,
	const unsigned long     repeat_offsets[],
	int                     total_offsets
);
extern void SDP_SetRepeatInterval(
	SDP_RepeatTime *   repeat_time,
	unsigned long      repeat_interval
);
extern void SDP_SetActiveDuration(
	SDP_RepeatTime *   repeat_time,
	unsigned long      active_duration
);
extern int SDP_SetRepeatOffsets(
	SDP_RepeatTime *      repeat_time,
	const unsigned long   repeat_offsets[],
	int                   total_offsets
);
extern void SDP_AddZoneAdjustment(
	SDP_Description *      description,
	SDP_ZoneAdjustment *   zone_adjustment
);
extern int SDP_AddNewZoneAdjustment(
	SDP_Description *   description,
	time_t              time,
	long                offset
);
extern void SDP_SetZoneAdjustmentTime(
	SDP_ZoneAdjustment *   zone_adjustment,
	time_t                 time
);
extern void SDP_SetZoneAdjustmentOffset(
	SDP_ZoneAdjustment *   zone_adjustment,
	long                   offset
);
extern int SDP_SetEncryption(
	SDP_Description *   description,
	const char *        method,
	const char *        key
);
extern int SDP_SetEncryptionMethod(
	SDP_Encryption *   encryption,
	const char *       method
);
extern int SDP_SetEncryptionKey(
	SDP_Encryption *   encryption,
	const char *       key
);
extern void SDP_AddAttribute(
	SDP_Description *   description,
	SDP_Attribute *     attribute
);
extern int SDP_AddNewAttribute(
	SDP_Description *   description,
	const char *        name,
	const char *        value
);
extern int SDP_SetAttributeName(
	SDP_Attribute *   attribute,
	const char *      name
);
extern int SDP_SetAttributeValue(
	SDP_Attribute *   attribute,
	const char *      value
);
extern void SDP_AddMediaDescription(
	SDP_Description *        description,
	SDP_MediaDescription *   media_description
);
extern int SDP_AddNewMediaDescription(
	SDP_Description *   description,
	const char *        media_type,
	unsigned short      port,
	unsigned short      total_ports,
	const char *        transport_protocol,
	const char *        formats,
	const char *        media_information
);
extern int SDP_SetMediaType(
	SDP_MediaDescription *   media_description,
	const char *             media_type
);
extern void SDP_SetMediaPort(
	SDP_MediaDescription *   media_description,
	unsigned short           port
);
extern void SDP_SetTotalMediaPorts(
	SDP_MediaDescription *   media_description,
	unsigned short           total_ports
);
extern int SDP_SetMediaTransportProtocol(
	SDP_MediaDescription *   media_description,
	const char *             transport_protocol
);
extern int SDP_SetMediaFormats(
	SDP_MediaDescription *   media_description,
	const char *             formats
);
extern int SDP_SetMediaInformation(
	SDP_MediaDescription *   media_description,
	const char *             media_information
);
extern int SDP_SetMediaConnection(
	SDP_MediaDescription *   media_description,
	const char *             network_type,
	const char *             address_type,
	const char *             address,
	int                      ttl,
	int                      total_addresses
);
extern int SDP_SetMediaBandwidth(
	SDP_MediaDescription *   media_description,
	const char *             modifier,
	long                     value
);
extern int SDP_SetMediaEncryption(
	SDP_MediaDescription *   media_description,
	const char *             method,
	const char *             key
);
extern void SDP_AddMediaAttribute(
	SDP_MediaDescription *   media_description,
	SDP_Attribute *          attribute
);
extern int SDP_AddNewMediaAttribute(
	SDP_MediaDescription *   media_description,
	const char *             name,
	const char *             value
);

extern int SDP_GetProtocolVersion(SDP_Description *description);
extern SDP_Owner *SDP_GetOwner(SDP_Description *description);
extern const char *SDP_GetUsername(SDP_Owner *owner);
extern const char *SDP_GetSessionID(SDP_Owner *owner);
extern const char *SDP_GetSessionVersion(SDP_Owner *owner);
extern const char *SDP_GetOwnerNetworkType(SDP_Owner *owner);
extern const char *SDP_GetOwnerAddressType(SDP_Owner *owner);
extern const char *SDP_GetOwnerAddress(SDP_Owner *owner);
extern const char *SDP_GetSessionName(SDP_Description *description);
extern const char *SDP_GetSessionInformation(SDP_Description *description);
extern const char *SDP_GetURI(SDP_Description *description);
extern SDP_EmailContact *SDP_GetEmailContacts(SDP_Description *description);
extern const char *SDP_GetEmailAddress(SDP_EmailContact *email_contact);
extern const char *SDP_GetEmailName(SDP_EmailContact *email_contact);
extern void SDP_RemoveEmailContact(
	SDP_Description *    description,
	SDP_EmailContact *   email_contact
);
extern SDP_PhoneContact *SDP_GetPhoneContacts(SDP_Description *description);
extern const char *SDP_GetPhoneNumber(SDP_PhoneContact *phone_contact);
extern const char *SDP_GetPhoneName(SDP_PhoneContact *phone_contact);
extern void SDP_RemovePhoneContact(
	SDP_Description *    description,
	SDP_PhoneContact *   phone_contact
);
extern SDP_Connection *SDP_GetConnection(SDP_Description *description);
extern const char *SDP_GetConnectionNetworkType(SDP_Connection *connection);
extern const char *SDP_GetConnectionAddressType(SDP_Connection *connection);
extern const char *SDP_GetConnectionAddress(SDP_Connection *connection);
extern int SDP_GetConnectionTTL(SDP_Connection *connection);
extern int SDP_GetTotalConnectionAddresses(SDP_Connection *connection);
extern SDP_Bandwidth *SDP_GetBandwidth(SDP_Description *description);
extern const char *SDP_GetBandwidthModifier(SDP_Bandwidth *bandwidth);
extern long SDP_GetBandwidthValue(SDP_Bandwidth *bandwidth);
extern SDP_SessionPlayTime *SDP_GetSessionPlayTimes(
	SDP_Description *description
);
extern time_t SDP_GetStartTime(SDP_SessionPlayTime *session_play_time);
extern time_t SDP_GetEndTime(SDP_SessionPlayTime *session_play_time);
extern SDP_RepeatTime *SDP_GetRepeatTimes(
	SDP_SessionPlayTime *session_play_time
);
extern unsigned long SDP_GetRepeatInterval(SDP_RepeatTime *repeat_time);
extern unsigned long SDP_GetActiveDuration(SDP_RepeatTime *repeat_time);
extern unsigned long *SDP_GetRepeatOffsets(SDP_RepeatTime *repeat_time);
extern int SDP_GetTotalRepeatOffsets(SDP_RepeatTime *repeat_time);
extern void SDP_RemoveRepeatTime(
	SDP_SessionPlayTime *   session_play_times,
	SDP_RepeatTime *        repeat_time
);
extern void SDP_RemoveSessionPlayTime(
	SDP_Description *       description,
	SDP_SessionPlayTime *   session_play_time
);
extern SDP_ZoneAdjustment *SDP_GetZoneAdjustments(SDP_Description *description);
extern time_t SDP_GetZoneAdjustmentTime(SDP_ZoneAdjustment *zone_adjustment);
extern long SDP_GetZoneAdjustmentOffset(SDP_ZoneAdjustment *zone_adjustment);
extern void SDP_RemoveZoneAdjustment(
	SDP_Description *      description,
	SDP_ZoneAdjustment *   zone_adjustment
);
extern SDP_Encryption *SDP_GetEncryption(SDP_Description *description);
extern const char *SDP_GetEncryptionMethod(SDP_Encryption *encryption);
extern const char *SDP_GetEncryptionKey(SDP_Encryption *encryption);
extern SDP_Attribute *SDP_GetAttributes(SDP_Description *description);
extern const char *SDP_GetAttributeName(SDP_Attribute *attribute);
extern const char *SDP_GetAttributeValue(SDP_Attribute *attribute);
extern void SDP_RemoveAttribute(
	SDP_Description *   description,
	SDP_Attribute *     attribute
);
extern SDP_MediaDescription *SDP_GetMediaDescriptions(
	SDP_Description *description
);
extern const char *SDP_GetMediaType(SDP_MediaDescription *media_description);
extern unsigned short SDP_GetMediaPort(SDP_MediaDescription *media_description);
extern unsigned short SDP_GetTotalMediaPorts(
	SDP_MediaDescription *media_description
);
extern const char *SDP_GetMediaTransportProtocol(
	SDP_MediaDescription *media_description
);
extern const char *SDP_GetMediaFormats(SDP_MediaDescription *media_description);
extern const char *SDP_GetMediaInformation(
	SDP_MediaDescription *media_description
);
extern SDP_Connection *SDP_GetMediaConnection(
	SDP_MediaDescription *media_description
);
extern SDP_Bandwidth *SDP_GetMediaBandwidth(
	SDP_MediaDescription *media_description
);
extern SDP_Encryption *SDP_GetMediaEncryption(
	SDP_MediaDescription *media_description
);
extern SDP_Attribute *SDP_GetMediaAttributes(
	SDP_MediaDescription *media_description
);
extern void SDP_RemoveMediaDescription(
	SDP_Description *        description,
	SDP_MediaDescription *   media_description
);

/* We also provide more descriptive versions of the Get/Set list macros: */
#define SDP_GetNextDescription          SDP_GetNext
#define SDP_GetPreviousDescription      SDP_GetPrevious
#define SDP_GetNextEmailContact         SDP_GetNext
#define SDP_GetPreviousEmailContact     SDP_GetPrevious
#define SDP_GetNextPhoneContact         SDP_GetNext
#define SDP_GetPreviousPhoneContact     SDP_GetPrevious
#define SDP_GetNextSessionPlayTime      SDP_GetNext
#define SDP_GetPreviousSessionPlayTime  SDP_GetPrevious
#define SDP_GetNextRepeatTime           SDP_GetNext
#define SDP_GetPreviousRepeatTime       SDP_GetPrevious
#define SDP_GetNextZoneAdjustment       SDP_GetNext
#define SDP_GetPreviousZoneAdjustment   SDP_GetPrevious
#define SDP_GetNextAttribute            SDP_GetNext
#define SDP_GetPreviousAttribute        SDP_GetPrevious
#define SDP_GetNextMediaDescription     SDP_GetNext

#define SDP_SetNextDescription          SDP_SetNext
#define SDP_SetPreviousDescription      SDP_SetPrevious
#define SDP_SetNextEmailContact         SDP_SetNext
#define SDP_SetPreviousEmailContact     SDP_SetPrevious
#define SDP_SetNextPhoneContact         SDP_SetNext
#define SDP_SetPreviousPhoneContact     SDP_SetPrevious
#define SDP_SetNextSessionPlayTime      SDP_SetNext
#define SDP_SetPreviousSessionPlayTime  SDP_SetPrevious
#define SDP_SetNextRepeatTime           SDP_SetNext
#define SDP_SetPreviousRepeatTime       SDP_SetPrevious
#define SDP_SetNextZoneAdjustment       SDP_SetNext
#define SDP_SetPreviousZoneAdjustment   SDP_SetPrevious
#define SDP_SetNextAttribute            SDP_SetNext
#define SDP_SetPreviousAttribute        SDP_SetPrevious
#define SDP_SetNextMediaDescription     SDP_SetNext

extern void SDP_DestroyDescriptions(SDP_Description *descriptions);
extern void SDP_DestroyDescription(SDP_Description *description);
extern void SDP_DestroyOwner(SDP_Owner *owner);
extern void SDP_DestroyEmailContacts(SDP_Description *description);
extern void SDP_DestroyEmailContact(SDP_EmailContact *email_contact);
extern void SDP_DestroyPhoneContacts(SDP_Description *description);
extern void SDP_DestroyPhoneContact(SDP_PhoneContact *phone_contact);
extern void SDP_DestroyConnection(SDP_Connection *connection);
extern void SDP_DestroyBandwidth(SDP_Bandwidth *bandwidth);
extern void SDP_DestroySessionPlayTimes(SDP_Description *description);
extern void SDP_DestroySessionPlayTime(SDP_SessionPlayTime *session_play_time);
extern void SDP_DestroyRepeatTimes(SDP_SessionPlayTime *session_play_time);
extern void SDP_DestroyRepeatTime(SDP_RepeatTime *repeat_time);
extern void SDP_DestroyZoneAdjustments(SDP_Description *description);
extern void SDP_DestroyZoneAdjustment(SDP_ZoneAdjustment *zone_adjustment);
extern void SDP_DestroyEncryption(SDP_Encryption *encryption);
extern void SDP_DestroyAttributes(SDP_Description *description);
extern void SDP_DestroyAttribute(SDP_Attribute *attribute);
extern void SDP_DestroyMediaDescriptions(SDP_Description *description);
extern void SDP_DestroyMediaDescription(SDP_MediaDescription *media_description);
extern void SDP_DestroyMediaAttributes(SDP_MediaDescription *media_description);





#ifdef __cplusplus
}
#endif

#endif
