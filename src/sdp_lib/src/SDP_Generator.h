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

#ifndef SDP_GENERATOR_INCLUDED
#define SDP_GENERATOR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <sys/types.h>
#include "SDP_Description.h"
#include "SDP_Error.h"
#include "SDP_LinkedList.h"
#include "SDP_Str.h"
#include "SDP_Utility.h"



typedef struct {
	/*
	 * A string struct containing a pointer to a malloc()'d buffer to store
	 * the SDP description as we generate it:
	 */
	SDP_Str output_buffer;
} SDP_Generator;



extern char *SDP_OutputDescriptionsToString(SDP_Description *descriptions);
extern char *SDP_OutputDescriptionToString(SDP_Description *description);
extern int SDP_OutputDescriptionsToFile(
	SDP_Description *   descriptions,
	const char *        filename
);
extern int SDP_OutputDescriptionToFile(
	SDP_Description *   description,
	const char *        filename
);

extern SDP_Generator *SDP_NewGenerator(void);

extern int SDP_GenProtocolVersionField(
	SDP_Generator *   generator,
	int               protocol_version
);
extern int SDP_GenOwnerField(
	SDP_Generator *   generator,
	const char *      username,
	const char *      session_id,
	const char *      session_version,
	const char *      network_type,
	const char *      address_type,
	const char *      address
);
extern int SDP_GenFromOwner(
	SDP_Generator *   generator,
	SDP_Owner *       owner
);
extern int SDP_GenSessionNameField(
	SDP_Generator *   generator,
	const char *      session_name
);
extern int SDP_GenInformationField(
	SDP_Generator *   generator,
	const char *      information
);
extern int SDP_GenURIField(
	SDP_Generator *   generator,
	const char *      uri
);
extern int SDP_GenEmailContactField(
	SDP_Generator *   generator,
	const char *      address,
	const char *      name
);
extern int SDP_GenFromEmailContacts(
	SDP_Generator *      generator,
	SDP_EmailContact *   email_contacts
);
extern int SDP_GenFromEmailContact(
	SDP_Generator *      generator,
	SDP_EmailContact *   email_contact
);
extern int SDP_GenPhoneContactField(
	SDP_Generator *   generator,
	const char *      number,
	const char *      name
);
extern int SDP_GenFromPhoneContacts(
	SDP_Generator *      generator,
	SDP_PhoneContact *   phone_contacts
);
extern int SDP_GenFromPhoneContact(
	SDP_Generator *      generator,
	SDP_PhoneContact *   phone_contact
);
extern int SDP_GenConnectionField(
	SDP_Generator *   generator,
	const char *      network_type,
	const char *      address_type,
	const char *      address,
	int               ttl,
	int               total_addresses
);
extern int SDP_GenFromConnection(
	SDP_Generator *    generator,
	SDP_Connection *   connection
);
extern int SDP_GenBandwidthField(
	SDP_Generator *   generator,
	const char *      modifier,
	long              value
);
extern int SDP_GenFromBandwidth(
	SDP_Generator *   generator,
	SDP_Bandwidth *   bandwidth
);
extern int SDP_GenSessionPlayTimeField(
	SDP_Generator *   generator,
	time_t            start_time,
	time_t            end_time
);
extern int SDP_GenFromSessionPlayTimes(
	SDP_Generator *         generator,
	SDP_SessionPlayTime *   session_play_times
);
extern int SDP_GenFromSessionPlayTime(
	SDP_Generator *         generator,
	SDP_SessionPlayTime *   session_play_time
);
extern int SDP_GenRepeatTimeField(
	SDP_Generator *   generator,
	const char *      repeat_interval,
	const char *      active_duration,
	const char *      repeat_offsets
);
extern int SDP_GenFromRepeatTimes(
	SDP_Generator *    generator,
	SDP_RepeatTime *   repeat_times
);
extern int SDP_GenFromRepeatTime(
	SDP_Generator *    generator,
	SDP_RepeatTime *   repeat_time
);
extern int SDP_GenZoneAdjustmentsField(
	SDP_Generator *   generator,
	int               total_adjustments,
	...
);
extern int SDP_GenFromZoneAdjustments(
	SDP_Generator *        generator,
	SDP_ZoneAdjustment *   zone_adjustments
);
extern int SDP_GenEncryptionField(
	SDP_Generator *   generator,
	const char *      method,
	const char *      key
);
extern int SDP_GenFromEncryption(
	SDP_Generator *    generator,
	SDP_Encryption *   encryption
);
extern int SDP_GenAttributeField(
	SDP_Generator *   generator,
	const char *      name,
	const char *      value
);
extern int SDP_GenFromAttributes(
	SDP_Generator *   generator,
	SDP_Attribute *   attributes
);
extern int SDP_GenFromAttribute(
	SDP_Generator *   generator,
	SDP_Attribute *   attribute
);
extern int SDP_GenMediaDescriptionField(
	SDP_Generator *   generator,
	const char *      media_type,
	const char *      port,
	const char *      transport_protocol,
	const char *      formats
);
extern int SDP_GenFromMediaDescriptions(
	SDP_Generator *          generator,
	SDP_MediaDescription *   media_descriptions
);
extern int SDP_GenFromMediaDescription(
	SDP_Generator *          generator,
	SDP_MediaDescription *   media_description
);

extern char *SDP_GetGeneratedOutput(SDP_Generator *generator);
extern int SDP_SaveGeneratedOutput(
	SDP_Generator *   generator,
	const char *      filename
);

extern void SDP_DestroyGenerator(SDP_Generator *generator);



#ifdef __cplusplus
}
#endif

#endif
