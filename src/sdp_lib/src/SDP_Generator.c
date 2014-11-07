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
 * This file contains routines that implement the session description
 * generator. See SDP_Generator.html for documentation.
 * 
 ******************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "SDP_Error.h"
#include "SDP_Description.h"
#include "SDP_Generator.h"
#include "SDP_Str.h"
#include "SDP_Utility.h"

/*
 * This is used to ensure _Generate() never gets passed a NULL pointer as the
 * replacement argument for a "%s" sequence in the format; always a real
 * pointer to *some* string, even just an empty one:
 */
#define STRINGIFY(_string_) ((_string_) ? _string_ : "")

/* This converts a time_t to a Network Time Protocol seconds timestamp: */
#define OS_TIME_TO_NETWORK_TIME(_time_) \
	((_time_) ? ((unsigned long) (_time_)) + 2208988800UL : 0UL)







static int _DescriptionAsString(
	SDP_Generator *     generator,
	SDP_Description *   description
);

char *SDP_OutputDescriptionsToString(SDP_Description *descriptions)
{
	SDP_Generator *generator;
	SDP_Description *description;
	char *generated_descriptions;
	int rv;

	SDP_AssertNotNull(descriptions);

	generator = SDP_NewGenerator();
	if (generator == NULL)
		return NULL;

	description = descriptions;
	while (description)
	{
		rv = _DescriptionAsString(generator, description);
		if (SDP_FAILED(rv))
		{
			SDP_DestroyGenerator(generator);
			return NULL;
		}

		description = SDP_GetNextDescription(description);
	}

	generated_descriptions = SDP_StrDup(SDP_GetGeneratedOutput(generator));

	SDP_DestroyGenerator(generator);

	return generated_descriptions;
}





char *SDP_OutputDescriptionToString(SDP_Description *description)
{
	SDP_Generator *generator;
	char *generated_description;
	int rv;
	
	SDP_AssertNotNull(description);

	generator = SDP_NewGenerator();
	if (generator == NULL)
		return NULL;

	rv = _DescriptionAsString(generator, description);
	if (SDP_FAILED(rv))
	{
		SDP_DestroyGenerator(generator);
		return NULL;
	}

	generated_description = SDP_StrDup(SDP_GetGeneratedOutput(generator));

	SDP_DestroyGenerator(generator);

	return generated_description;
}





int SDP_OutputDescriptionsToFile(
	SDP_Description *   descriptions,
	const char *        filename)
{
	char *string;
	FILE *file;

	SDP_AssertNotNull(descriptions);
	SDP_AssertNotNull(filename);

	string = SDP_OutputDescriptionsToString(descriptions);
	if (string == NULL)
		return SDP_FAILURE;

	file = fopen(filename, "w");
	if (file == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_FILE_OPEN_FAILED,
			"Couldn't open file to output descriptions to: %s.",
			SDP_OS_ERROR_STRING
		);
		SDP_Destroy(string);
		return SDP_FAILURE;
	}

	fwrite(string, 1, strlen(string), file);

	fclose(file);
	SDP_Destroy(string);

	return SDP_SUCCESS;
}





int SDP_OutputDescriptionToFile(
	SDP_Description *   description,
	const char *        filename)
{
	char *string;
	FILE *file;

	SDP_AssertNotNull(description);
	SDP_AssertNotNull(filename);

	string = SDP_OutputDescriptionToString(description);
	if (string == NULL)
		return SDP_FAILURE;

	file = fopen(filename, "w");
	if (file == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_FILE_OPEN_FAILED,
			"Couldn't open file to output description to: %s.",
			SDP_OS_ERROR_STRING
		);
		SDP_Destroy(string);
		return SDP_FAILURE;
	}

	fwrite(string, 1, strlen(string), file);

	fclose(file);
	SDP_Destroy(string);

	return SDP_SUCCESS;
}





/* This adds formatted strings to the generator output buffer: */
static int _Generate(
	SDP_Generator *   generator,
	const char *      format,
	...
);

SDP_Generator *SDP_NewGenerator(void)
{
	SDP_Generator *generator =
		(SDP_Generator *) SDP_Allocate(sizeof(SDP_Generator));
	if (generator == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to create a new SDP "
			"generator: %s.",
			SDP_OS_ERROR_STRING
		);
	}

	memset(generator, 0, sizeof(SDP_Generator));

	return generator;
}





int SDP_GenProtocolVersionField(
	SDP_Generator *   generator,
	int               protocol_version)
{
	SDP_AssertNotNull(generator);

	return _Generate(generator, "v=%d\n", protocol_version);
}





int SDP_GenOwnerField(
	SDP_Generator *   generator,
	const char *      username,
	const char *      session_id,
	const char *      session_version,
	const char *      network_type,
	const char *      address_type,
	const char *      address)
{
	SDP_AssertNotNull(generator);

	return _Generate(
		generator,
		"o=%s %s %s %s %s %s\n",
		username ? username : "-",
		STRINGIFY(session_id),
		session_version ? session_version : "0",
		STRINGIFY(network_type),
		STRINGIFY(address_type),
		STRINGIFY(address)
	);
}





int SDP_GenFromOwner(
	SDP_Generator *   generator,
	SDP_Owner *       owner)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(owner);

	return SDP_GenOwnerField(
		generator,
		SDP_GetUsername(owner),
		SDP_GetSessionID(owner),
		SDP_GetSessionVersion(owner),
		SDP_GetOwnerNetworkType(owner),
		SDP_GetOwnerAddressType(owner),
		SDP_GetOwnerAddress(owner)
	);
}





int SDP_GenSessionNameField(
	SDP_Generator *   generator,
	const char *      session_name)
{
	SDP_AssertNotNull(generator);

	return _Generate(generator, "s=%s\n", STRINGIFY(session_name));
}





int SDP_GenInformationField(
	SDP_Generator *   generator,
	const char *      information)
{
	SDP_AssertNotNull(generator);

	return _Generate(generator, "i=%s\n", STRINGIFY(information));
}





int SDP_GenURIField(
	SDP_Generator *   generator,
	const char *      uri)
{
	SDP_AssertNotNull(generator);

	return _Generate(generator, "u=%s\n", STRINGIFY(uri));
}





int SDP_GenEmailContactField(
	SDP_Generator *   generator,
	const char *      address,
	const char *      name)
{
	SDP_AssertNotNull(generator);

	if (name)
		return _Generate(
			generator, "e=%s <%s>\n", name, STRINGIFY(address)
		);
	else
		return _Generate(generator, "e=%s\n", STRINGIFY(address));
}





int SDP_GenFromEmailContacts(
	SDP_Generator *      generator,
	SDP_EmailContact *   email_contacts)
{
	SDP_EmailContact *email_contact;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(email_contacts);

	email_contact = email_contacts;
	while (email_contact)
	{
		rv = SDP_GenFromEmailContact(generator, email_contact);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		email_contact = SDP_GetNextEmailContact(email_contact);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromEmailContact(
	SDP_Generator *      generator,
	SDP_EmailContact *   email_contact)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(email_contact);

	return SDP_GenEmailContactField(
		generator,
		SDP_GetEmailAddress(email_contact),
		SDP_GetEmailName(email_contact)
	);
}






int SDP_GenPhoneContactField(
	SDP_Generator *   generator,
	const char *      number,
	const char *      name)
{
	SDP_AssertNotNull(generator);

	if (name)
		return _Generate(
			generator,
			"p=%s (%s)\n",
			STRINGIFY(number),
			name
		);
	else
		return _Generate(generator, "p=%s\n", STRINGIFY(number));
}





int SDP_GenFromPhoneContacts(
	SDP_Generator *      generator,
	SDP_PhoneContact *   phone_contacts)
{
	SDP_PhoneContact *phone_contact;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(phone_contacts);

	phone_contact = phone_contacts;
	while (phone_contact)
	{
		rv = SDP_GenFromPhoneContact(generator, phone_contact);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		phone_contact = SDP_GetNextPhoneContact(phone_contact);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromPhoneContact(
	SDP_Generator *      generator,
	SDP_PhoneContact *   phone_contact)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(phone_contact);

	return SDP_GenPhoneContactField(
		generator,
		SDP_GetPhoneNumber(phone_contact),
		SDP_GetPhoneName(phone_contact)
	);
}





int SDP_GenConnectionField(
	SDP_Generator *   generator,
	const char *      network_type,
	const char *      address_type,
	const char *      address,
	int               ttl,
	int               total_addresses)
{
	SDP_AssertNotNull(generator);

	if (ttl && total_addresses)
		return _Generate(
			generator,
			"c=%s %s %s/%d/%d\n",
			STRINGIFY(network_type),
			STRINGIFY(address_type),
			STRINGIFY(address),
			ttl,
			total_addresses
		);
	else if (ttl)
		return _Generate(
			generator,
			"c=%s %s %s/%d\n",
			STRINGIFY(network_type),
			STRINGIFY(address_type),
			STRINGIFY(address),
			ttl
		);
	else
		return _Generate(
			generator,
			"c=%s %s %s\n",
			STRINGIFY(network_type),
			STRINGIFY(address_type),
			STRINGIFY(address)
		);
}





int SDP_GenFromConnection(
	SDP_Generator *    generator,
	SDP_Connection *   connection)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(connection);

	return SDP_GenConnectionField(
		generator,
		SDP_GetConnectionNetworkType(connection),
		SDP_GetConnectionAddressType(connection),
		SDP_GetConnectionAddress(connection),
		SDP_GetConnectionTTL(connection),
		SDP_GetTotalConnectionAddresses(connection)
	);
}





int SDP_GenBandwidthField(
	SDP_Generator *   generator,
	const char *      modifier,
	long              value)
{
	SDP_AssertNotNull(generator);

	return _Generate(
		generator,
		"b=%s:%ld\n",
		STRINGIFY(modifier),
		value
	);
}





int SDP_GenFromBandwidth(
	SDP_Generator *   generator,
	SDP_Bandwidth *   bandwidth)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(bandwidth);

	return SDP_GenBandwidthField(
		generator,
		SDP_GetBandwidthModifier(bandwidth),
		SDP_GetBandwidthValue(bandwidth)
	);
}





int SDP_GenSessionPlayTimeField(
	SDP_Generator *   generator,
	time_t            start_time,
	time_t            end_time)
{
	SDP_AssertNotNull(generator);

	return _Generate(
		generator,
		"t=%lu %lu\n",
		OS_TIME_TO_NETWORK_TIME(start_time),
		OS_TIME_TO_NETWORK_TIME(end_time)
	);
}





int SDP_GenFromSessionPlayTimes(
	SDP_Generator *         generator,
	SDP_SessionPlayTime *   session_play_times)
{
	SDP_SessionPlayTime *session_play_time;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(session_play_times);

	session_play_time = session_play_times;
	while (session_play_time)
	{
		rv = SDP_GenFromSessionPlayTime(
			generator, session_play_time
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		session_play_time =
			SDP_GetNextSessionPlayTime(session_play_time);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromSessionPlayTime(
	SDP_Generator *         generator,
	SDP_SessionPlayTime *   session_play_time)
{
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(session_play_time);

	rv = SDP_GenSessionPlayTimeField(
		generator,
		SDP_GetStartTime(session_play_time),
		SDP_GetEndTime(session_play_time)
	);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (SDP_GetRepeatTimes(session_play_time))
	{
		rv = SDP_GenFromRepeatTimes(
			generator,
			SDP_GetRepeatTimes(session_play_time)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	return SDP_SUCCESS;
}





int SDP_GenRepeatTimeField(
	SDP_Generator *   generator,
	const char *      repeat_interval,
	const char *      active_duration,
	const char *      repeat_offsets)
{
	int rv;

	SDP_AssertNotNull(generator);

	rv = _Generate(
		generator,
		"r=%s %s",
		STRINGIFY(repeat_interval),
		STRINGIFY(active_duration)
	);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (repeat_offsets && *repeat_offsets)
	{
		rv = _Generate(generator, " %s", repeat_offsets);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	rv = _Generate(generator, "\n");
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	return SDP_SUCCESS;
}





int SDP_GenFromRepeatTimes(
	SDP_Generator *    generator,
	SDP_RepeatTime *   repeat_times)
{
	SDP_RepeatTime *repeat_time;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(repeat_times);

	repeat_time = repeat_times;
	while (repeat_time)
	{
		rv = SDP_GenFromRepeatTime(generator, repeat_time);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		repeat_time = SDP_GetNextRepeatTime(repeat_time);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromRepeatTime(
	SDP_Generator *    generator,
	SDP_RepeatTime *   repeat_time)
{
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(repeat_time);

	rv = _Generate(
		generator,
		"%lu %lu",
		SDP_GetRepeatInterval(repeat_time),
		SDP_GetActiveDuration(repeat_time)
	);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (SDP_GetTotalRepeatOffsets(repeat_time))
	{
		int total_offsets;
		unsigned long *offsets;
		int i;

		total_offsets = SDP_GetTotalRepeatOffsets(repeat_time);
		offsets       = SDP_GetRepeatOffsets(repeat_time);
		for (i = 0; i < total_offsets; ++i)
		{
			rv = _Generate(generator, " %lu", offsets[i]);
			if (SDP_FAILED(rv))
				return SDP_FAILURE;
		}
	}

	return SDP_SUCCESS;
}





int SDP_GenZoneAdjustmentsField(
	SDP_Generator *   generator,
	int               total_adjustments,
	...)
{
	int rv;

	rv = _Generate(generator, "z=");
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (total_adjustments)
	{
		va_list zone_adjustments;
		int i;

		va_start(zone_adjustments, total_adjustments);
		for (i = 0; i < total_adjustments; ++i)
		{
			rv = _Generate(
				generator,
				" %lu %s",
				OS_TIME_TO_NETWORK_TIME(
					va_arg(zone_adjustments, long)
				),
				va_arg(zone_adjustments, char *)
			);
			if (SDP_FAILED(rv))
				return SDP_FAILURE;
		}
		va_end(zone_adjustments);
	}

	rv = _Generate(generator, "\n");
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	return SDP_SUCCESS;
}





int SDP_GenFromZoneAdjustments(
	SDP_Generator *        generator,
	SDP_ZoneAdjustment *   zone_adjustments)
{
	SDP_ZoneAdjustment *zone_adjustment;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(zone_adjustments);

	rv = _Generate(generator, "z=");
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	zone_adjustment = zone_adjustments;
	while (zone_adjustment)
	{
		rv = _Generate(
			generator,
			"%lu %ld",
			(unsigned long) SDP_GetZoneAdjustmentTime(
				zone_adjustment
			),
			SDP_GetZoneAdjustmentOffset(zone_adjustment)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		zone_adjustment = SDP_GetNextZoneAdjustment(zone_adjustment);

		/*
		 * Add the space to separate this adjustment from the next one:
		 */
		if (zone_adjustment)
		{
			rv = _Generate(generator, " ");
			if (SDP_FAILED(rv))
				return SDP_FAILURE;
		}
	}

	return SDP_SUCCESS;
}





int SDP_GenEncryptionField(
	SDP_Generator *   generator,
	const char *      method,
	const char *      key)
{
	SDP_AssertNotNull(generator);

	if (key)
		return _Generate(
			generator, "e=%s:%s\n", STRINGIFY(method), key
		);
	else
		return _Generate(generator, "e=%s\n", STRINGIFY(method));
}





int SDP_GenFromEncryption(
	SDP_Generator *    generator,
	SDP_Encryption *   encryption)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(encryption);

	return SDP_GenEncryptionField(
		generator,
		SDP_GetEncryptionMethod(encryption),
		SDP_GetEncryptionKey(encryption)
	);
}





int SDP_GenAttributeField(
	SDP_Generator *   generator,
	const char *      name,
	const char *      value)
{
	SDP_AssertNotNull(generator);

	if (value)
		return _Generate(
			generator, "a=%s:%s\n", STRINGIFY(name), value
		);
	else
		return _Generate(generator, "a=%s\n", STRINGIFY(name));
}





int SDP_GenFromAttributes(
	SDP_Generator *   generator,
	SDP_Attribute *   attributes)
{
	SDP_Attribute *attribute;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(attributes);

	attribute = attributes;
	while (attribute)
	{
		rv = SDP_GenFromAttribute(generator, attribute);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		attribute = SDP_GetNextAttribute(attribute);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromAttribute(
	SDP_Generator *   generator,
	SDP_Attribute *   attribute)
{
	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(attribute);

	return SDP_GenAttributeField(
		generator,
		SDP_GetAttributeName(attribute),
		SDP_GetAttributeValue(attribute)
	);
}





int SDP_GenMediaDescriptionField(
	SDP_Generator *   generator,
	const char *      media_type,
	const char *      port,
	const char *      transport_protocol,
	const char *      formats)
{
	if (formats)
		return _Generate(
			generator,
			"m=%s %s %s %s\n",
			STRINGIFY(media_type),
			STRINGIFY(port),
			STRINGIFY(transport_protocol),
			formats
		);
	else
		return _Generate(
			generator,
			"m=%s %s %s\n",
			STRINGIFY(media_type),
			STRINGIFY(port),
			STRINGIFY(transport_protocol)
		);
}





int SDP_GenFromMediaDescriptions(
	SDP_Generator *          generator,
	SDP_MediaDescription *   media_descriptions)
{
	SDP_MediaDescription *media_description;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(media_descriptions);

	media_description = media_descriptions;
	while (media_description)
	{
		rv = SDP_GenFromMediaDescription(generator, media_description);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;

		media_description =
			SDP_GetNextMediaDescription(media_description);
	}

	return SDP_SUCCESS;
}





int SDP_GenFromMediaDescription(
	SDP_Generator *          generator,
	SDP_MediaDescription *   media_description)
{
	char port_string[64];
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(media_description);

	if (SDP_GetMediaPort(media_description)
		&& SDP_GetTotalMediaPorts(media_description))
		snprintf(
			port_string,
			sizeof(port_string),
			"%d/%d",
			SDP_GetMediaPort(media_description),
			SDP_GetTotalMediaPorts(media_description)
		);
	else
		snprintf(
			port_string,
			sizeof(port_string),
			"%d",
			SDP_GetMediaPort(media_description)
		);

	rv = SDP_GenMediaDescriptionField(
		generator,
		SDP_GetMediaType(media_description),
		port_string,
		SDP_GetMediaTransportProtocol(media_description),
		SDP_GetMediaFormats(media_description)
	);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (SDP_GetMediaInformation(media_description))
	{
		rv = SDP_GenInformationField(
			generator, SDP_GetMediaInformation(media_description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetMediaConnection(media_description))
	{
		rv = SDP_GenFromConnection(
			generator, SDP_GetMediaConnection(media_description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetMediaBandwidth(media_description))
	{
		rv = SDP_GenFromBandwidth(
			generator, SDP_GetMediaBandwidth(media_description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetMediaAttributes(media_description))
	{
		rv = SDP_GenFromAttributes(
			generator, SDP_GetMediaAttributes(media_description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	return SDP_SUCCESS;
}





char *SDP_GetGeneratedOutput(SDP_Generator *generator)
{
	SDP_AssertNotNull(generator);

	return SDP_GetStrBuffer(generator->output_buffer);
}





int SDP_SaveGeneratedOutput(
	SDP_Generator *   generator,
	const char *      filename)
{
	FILE *file;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(filename);

	file = fopen(filename, "w");
	if (file == NULL)
	{
		SDP_RaiseFatalError(
			SDP_ERR_FILE_OPEN_FAILED,
			"Couldn't open file \"%s\" to save the generated SDP "
			"description: %s.",
			filename,
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	fwrite(
		SDP_GetStrBuffer(generator->output_buffer),
		1,
		SDP_GetStrLength(generator->output_buffer),
		file
	);

	fclose(file);

	return SDP_SUCCESS;
}





void SDP_DestroyGenerator(SDP_Generator *generator)
{
	SDP_DestroyStrBuffer(&generator->output_buffer);

	SDP_Destroy(generator);
}





static unsigned long _ApproximateLength(
	const char *   format,
	va_list        args
);

/*******************************************************************************
 *
 * 	Name
 * 		_Generate(generator, format, ...)
 *
 * 	Purpose
 * 		This function "generates"; it adds a formatted string to to the
 * 		SDP_Generator struct's output buffer, allocating memory as
 * 		needed.
 *
 * 		It returns true if it successfully adds the formatted string to
 * 		the buffer, false otherwise.
 * 
 * 	Parameters
 * 		generator - A pointer an SDP_Generator struct.
 * 		format    - A printf() format string.
 * 		...       - The variables to fill the string with.
 * 
 */

static int _Generate(
	SDP_Generator *   generator,
	const char *      format,
	...)
{
	va_list args;
	size_t bytes_needed;
	int rv;

	SDP_AssertNotNull(generator);
	SDP_AssertNotNull(format);

	/*
	 * Figure out how many bytes we're gonna need for this formatted
	 * string:
	 */
	va_start(args, format);
	bytes_needed = _ApproximateLength(format, args);
	va_end(args);

	/* Now add the formatted string to the buffer: */
	va_start(args, format);
	rv = SDP_CatToStrUsingFormat(
		&generator->output_buffer, bytes_needed, format, args
	);
	va_end(args);

	if (SDP_FAILED(rv))
	{
		SDP_RaiseFatalError(
			SDP_ERR_OUT_OF_MEMORY,
			"Couldn't allocate memory to generate SDP "
			"description: %s.",
			SDP_OS_ERROR_STRING
		);
		return SDP_FAILURE;
	}

	return SDP_SUCCESS;
}





/*******************************************************************************
 *
 * 	Name
 * 		_ApproximateLength(format, args)
 *
 * 	Purpose
 * 		Takes a format string and a va_list of arguments to fill it
 * 		with. It tries to find out how many bytes would be needed for
 * 		a snprintf() with that format and those arguments to succeed
 * 		without cutting off the resulting string, and returns that
 * 		value. It's an approximation, and a liberal one at that, so it
 * 		*shouldn't* return a value too small, but it *probably* will
 * 		return a value too big.
 *
 * 		C99 says that if snprintf() truncates the string, it returns
 * 		the number of bytes that *would* have been written to if enough
 * 		space was available, but sadly, snprintf() implementations
 * 		differ wildly in what they return, where snprintf() exist at
 * 		all, e.g.:
 * 
 * 		"The glibc implementation of the functions snprintf and
 * 		vsnprintf . . . . [u]ntil glibc 2.0.6 . . . would return -1
 * 		when the output was truncated." - man 3 snprintf
 *
 * 		So this function is needed. Otherwise we need to implement
 * 		snprintf() on our own or try to port it, which is overkill.
 * 
 * 	Parameters
 * 		format - A snprintf() format string.
 * 		args   - A va_list.
 * 
 */

#define NO_MODIFIERS 0
#define IS_LONG      1
#define IS_SHORT     2

static unsigned long _ApproximateLength(
	const char *   format,
	va_list        args)
{
	unsigned long length = 0;

	while (*format)
	{
		int modifiers = NO_MODIFIERS;

		if (*format != '%')
		{
			++format;
			++length;
			continue;
		}

		if (strncmp(format, "%%", 2) == 0)
		{
			format += 2;
			length += 1;
			continue;
		}

		++format;
		if (*format == '\0')
		{
			++length;
			break;
		}

		/* Check for short and long modifiers: */
		if (*format == 'h' || *format == 'H')
		{
			modifiers |= IS_SHORT;
			++format;
		}
		else if (*format == 'l' || *format == 'L')
		{
			modifiers |= IS_LONG;
			++format;
		}

		switch (*format)
		{
			case 'c':
			case 'C':
				++length;
				++format;
				va_arg(args, int);
				continue;
			case 's':
			case 'S':
				length += strlen(va_arg(args, char *));
				++format;
				continue;
			case 'u':
			case 'U':
			case 'd':
			case 'D':
			case 'i':
			case 'I':
				/*
				 * A cop-out. This is far, far, more than
				 * enough room, even for a 64 bit int:
				 */
				length += 24;
				++format;
				va_arg(args, int);
				continue;
			default:
				++length;
				++format;
				va_arg(args, int);
				continue;
		}
	}

	++length;

	return length;
}





/*******************************************************************************
 *
 * 	Name
 * 		_DescriptionAsString(generator, description)
 *
 * 	Purpose
 * 		This function "generates"; it adds a formatted string to to the
 * 		SDP_Generator struct's output buffer, allocating memory as
 * 		needed.
 *
 * 		It returns true if it successfully adds the formatted string to
 * 		the buffer, false otherwise.
 * 
 * 	Parameters
 * 		generator   - A pointer to the SDP_Generator struct to generate
 * 		              to.
 * 		description - A pointer to the SDP_Description struct to
 * 		              generate from.
 *
 */

static int _DescriptionAsString(
	SDP_Generator *     generator,
	SDP_Description *   description)
{
	int rv;

	rv = SDP_GenProtocolVersionField(
		generator, SDP_GetProtocolVersion(description)
	);
	if (SDP_FAILED(rv))
		return SDP_FAILURE;

	if (SDP_GetOwner(description))
	{
		rv = SDP_GenFromOwner(
			generator, SDP_GetOwner(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetSessionName(description))
	{
		rv = SDP_GenSessionNameField(
			generator, SDP_GetSessionName(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetSessionInformation(description))
	{
		rv = SDP_GenInformationField(
			generator,
			SDP_GetSessionInformation(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetURI(description))
	{
		rv = SDP_GenURIField(generator, SDP_GetURI(description));
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetEmailContacts(description))
	{
		rv = SDP_GenFromEmailContacts(
			generator, SDP_GetEmailContacts(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetPhoneContacts(description))
	{
		rv = SDP_GenFromPhoneContacts(
			generator, SDP_GetPhoneContacts(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetConnection(description))
	{
		rv = SDP_GenFromConnection(
			generator, SDP_GetConnection(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetBandwidth(description))
	{
		rv = SDP_GenFromBandwidth(
			generator, SDP_GetBandwidth(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetSessionPlayTimes(description))
	{
		rv = SDP_GenFromSessionPlayTimes(
			generator, SDP_GetSessionPlayTimes(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetZoneAdjustments(description))
	{
		rv = SDP_GenFromZoneAdjustments(
			generator, SDP_GetZoneAdjustments(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetAttributes(description))
	{
		rv = SDP_GenFromAttributes(
			generator, SDP_GetAttributes(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	if (SDP_GetMediaDescriptions(description))
	{
		rv = SDP_GenFromMediaDescriptions(
			generator, SDP_GetMediaDescriptions(description)
		);
		if (SDP_FAILED(rv))
			return SDP_FAILURE;
	}

	return SDP_SUCCESS;
}
