/* $Id: metadata.c,v 1.1.1.1 2003/09/03 12:45:42 chneuman Exp $ */
/*
 *  Copyright (c) 1999-2003 INRIA - Universite Paris 6 - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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


/*
 * metadata.c
 *
 *	Compute Metadata infos for fcast trailers.
 */
#include "fcast.h"
#include <string.h>

void AddMetadata(trailer_t *trailer, char *MetaName, char* MetaContent)
{	unsigned int MetaLen;

	MetaLen =  strlen(MetaName) + strlen(MetaContent) + 3;
	if (!(trailer->buffer = (char*)realloc(trailer->buffer, trailer->size + MetaLen)))
	{
		EXIT(("Error: Cannot alloc memory!\n"))
	}

	memcpy( trailer->buffer + trailer->size, MetaName, strlen(MetaName));
	trailer->size += strlen(MetaName);
	*(trailer->buffer+trailer->size) = ':';
	trailer->size ++;
	memcpy( trailer->buffer + trailer->size, MetaContent, strlen(MetaContent));
	trailer->size += strlen(MetaContent);
	*(trailer->buffer + trailer->size) = '\r';
	trailer->size ++;
	*(trailer->buffer + trailer->size) = '\n';
	trailer->size ++;
}


void EndTrailer(trailer_t *trailer)
{	int final_size = 0;

	if (!(trailer->buffer = (char*)realloc(trailer->buffer, trailer->size + 4) ))
	{
		EXIT(("Error: Cannot alloc memory!\n"))

	}
	final_size = trailer->size; /* do not count the size field itself */

	*(trailer->buffer + trailer->size) = (final_size & 0xFF000000) >> 24;
	trailer->size++;
	*(trailer->buffer + trailer->size) = (final_size & 0x00FF0000) >> 16;
	trailer->size++;
	*(trailer->buffer + trailer->size) = (final_size & 0x0000FF00) >> 8;
	trailer->size++;
	*(trailer->buffer + trailer->size) = (final_size & 0x0000FF);
	trailer->size++;
}

void DeleteTrailer(trailer_t *trailer)
{
	free(trailer->buffer);
	trailer->size = 0;
	trailer->buffer = NULL;
	
}


void AddMetaToList(meta_t **MetaList, meta_t NewMeta)
{		
	if (*MetaList == NULL) {
		if (!(*MetaList = (meta_t *)malloc(sizeof(meta_t)))) {
			EXIT(("Error: Cannot alloc memory!\n"))
		}
		strncpy((*MetaList)->name, NewMeta.name,MAX_NAMELEN );
		strncpy((*MetaList)->content, NewMeta.content,MAX_CONTLEN );
		(*MetaList)->value = NewMeta.value;
		(*MetaList)->next = NULL;
	}	
	else {
		meta_t *List = *MetaList;
		while (List->next != NULL) {
			List = List->next;
		}
		if (!(List->next = (meta_t *)malloc(sizeof(meta_t)))) {
			EXIT(("Error: Cannot alloc memory!\n"))
		}
		List = List->next;
		strcpy(List->name, NewMeta.name);
		strcpy(List->content, NewMeta.content);
		List->value = NewMeta.value;
		List->next = NULL;
	}
}


meta_t*	ParseTrailer(char *TrailerBuff)
{
	meta_t *MetaList = NULL;
	meta_t NewMeta;	
	char *name, *content;
	
	name = strtok( TrailerBuff, ":" );

	while( name != NULL ) /* while there are Metadatas in the buffer */
	{
		strncpy(NewMeta.name , name, MAX_NAMELEN);
		content = strtok( NULL, "\n" );
		if(content == NULL) {
			printf("Invalid Trailer!\n");
			goto failed;
		}
		if(content[strlen(content)-1] == '\r')
			content[strlen(content)-1] = '\0';
	
		strncpy(NewMeta.content , content, MAX_CONTLEN);

		if (isdigit((int)*content))
			NewMeta.value = atol(content);
		else
			NewMeta.value = 0;

		AddMetaToList( &MetaList, NewMeta);
		name = strtok( NULL, ":" );
	}

	return MetaList;

failed: return NULL;
	
}

meta_t* FindMeta(meta_t *MetaList, char *MetaName)
{
	meta_t *found = NULL;
	meta_t *listloop = MetaList;
	
	while(listloop != NULL)
	{
		if( !strcmp(listloop->name, MetaName) ) {
			found = listloop;
			break;
		}
		listloop = listloop->next;
	}
	return found;
}



void	DestroyMetalist(meta_t *MetaList)
{
	meta_t *looplist = MetaList;
	meta_t *toDelete;

	while (looplist != NULL)
	{
		toDelete = looplist;
		looplist = looplist->next;
		free(toDelete);
	}
	
}


void	PrintMetaList(meta_t *MetaList)
{
	meta_t	*lpMetaData = MetaList;
	while(lpMetaData != NULL)
	{
		PRINT(("\t%s: %s\n", lpMetaData->name, lpMetaData->content))
		lpMetaData = lpMetaData->next;
	}
}


void GetFilePathFromMeta (meta_t *MetaList, char *FullFilePath)
{
	char		file_base[MAX_PATH];
	char		file_name[MAX_FILENAME];
	meta_t *lpMetaData = NULL;

	memset((void *)file_base, 0, MAX_PATH);
	memset((void *)file_name, 0, MAX_FILENAME);

	lpMetaData = FindMeta(MetaList, "Content-Location");
	if(lpMetaData == NULL)
	{
		EXIT(("Meta Content-location unavailable, file name is unknown, can't save...\n"))
	}
	strncpy(file_name, lpMetaData->content, strlen(lpMetaData->content));
	if((lpMetaData = FindMeta(MetaList, "Content-Base"))!=NULL )
	{
		char filebasetmp[MAX_PATH];
                ASSERT(strlen(lpMetaData->content)>1);
		memset((void *)filebasetmp, 0, MAX_PATH);

		strncpy(filebasetmp, lpMetaData->content, strlen(lpMetaData->content));
		filebasetmp[strlen(lpMetaData->content)] = 0;

		if( filebasetmp[0] != '.' )
		{
			if( filebasetmp[0] != '/' && filebasetmp[0] != '\\' && filebasetmp[1] != ':')
			{
#ifdef WIN32
				strcpy(file_base, ".\\");
#else
				strcpy(file_base, "./");
#endif
			}

			else
			{
				strcpy(file_base, "./");
			}
		}
		if(filebasetmp[1] == ':')
		{
			strcat(file_base, (filebasetmp+2));
		}
		else
		{
			strcat(file_base, filebasetmp);
		}
	}
	else
	{
#ifdef WIN32
		strcpy(file_base, ".\\\0");
#else
		strcpy(file_base, "./\0");
#endif
	}

	memset((void*)FullFilePath, 0, MAX_FILENAME + MAX_PATH);
	strcpy(FullFilePath, file_base);
	strcat(FullFilePath, file_name);
}


long GetMetaLength(meta_t *MetaList)
{
	meta_t *lpMetaData = FindMeta(MetaList, "Content-Length");
	if(lpMetaData == NULL)
	{
		EXIT(("Error: Missing Meta Content-Length"))
	}
	return lpMetaData->value;
}


long GetMetaFilesize(meta_t *MetaList)
{
	meta_t *lpMetaData = FindMeta(MetaList, "Content-Filesize");
	if(lpMetaData == NULL)
	{
		EXIT(("Error: Missing Meta Content-Filesize"))
	}
	return lpMetaData->value;
}


int GetMetaFragment(meta_t *MetaList, int *FragIndice, int *NbFragTot)
{
	meta_t *lpMetaData = FindMeta(MetaList, "Content-Fragment");
	if(lpMetaData == NULL)
	{
		EXIT(("Error: Meta Content-Fragment unavailable\n"))
	}
	else
	{	char *token;
		token = strtok( lpMetaData->content, "/" );
		if( token != NULL )
		{
			*FragIndice = atoi(token);
			token = strtok( NULL, "/" );
			if( token != NULL )
				*NbFragTot = atoi(token);
			else
				EXIT(("Error: Invalide Meta (Content-Fragment)\n"))
		}
		else
			EXIT(("Error: Invalid Meta (Content-Fragment)\n"))
	}
	return *NbFragTot;
}


long GetMetaOffset(meta_t *MetaList)
{
	meta_t *lpMetaData = FindMeta(MetaList, "Content-Offset");
	if(lpMetaData == NULL)
	{
		EXIT(("Error: Missing Meta Content-Offset"))
	}
	return lpMetaData->value;
}

