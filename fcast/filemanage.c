/* $Id: filemanage.c,v 1.2 2004/05/26 07:36:03 roca Exp $ */
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
 * filemanage.c
 *
 *	Tools for Directories listing and files management...
 */

#include "fcast.h"


int FileExist( const char *filepath)
{
	FILE *check;
	check = fopen(filepath, "r");
	
	ASSERT(filepath!=NULL);

	if( check == NULL)
		return 0;
	else
	{
		fclose(check);
		return 1;
	}

}



int IsDirDots (const char *path)
{
  return (path[0] == '\0'
	  || (path[0] == '.' && (path[1] == '\0'
			      || (path[1] == '.' && path[2] == '\0'))));
}



void GetFileName( char *filepath, char *filename )
{
	char *token1, *token2;

	ASSERT(    (filepath != NULL) 
		&& (filename != NULL) 
		&& (strlen(filepath)<=(MAX_PATH+MAX_FILENAME)) )

	token1 = filepath;
	token2 = strpbrk(token1, "/\\");
	while( token2 )
	{
		token1 = token2+1;
		token2 = strpbrk(token1, "/\\");
	}
	strncpy(filename, token1, MAX_FILENAME);
}


void GetFileBase( char *filepath, char *filebase )
{
	char *token1, *token2;

	ASSERT(    (filepath != NULL) 
		&& (filebase != NULL) 
		&& (strlen(filepath)<=(MAX_PATH+MAX_FILENAME)) )

	token1 = filepath;
	token2 = strpbrk(token1, "/\\");
	while( token2 )
	{
		token1 = token2+1;
		token2 = strpbrk(token1, "/\\");
	}
	if(token1 == filepath)
		strcpy(filebase, "./");
	else
	{
		strncpy(filebase, filepath, (strlen(filepath)-strlen(token1)));
		filebase[strlen(filepath)-strlen(token1)] = '\0';
	}
	
}



void GetFileBaseWithoutPrefix( char *filepath, char *filebase )
{
	char *token1, *token2;
	unsigned int path_prefix = 0;
	unsigned int BaseLen = 0;

	ASSERT(    (filepath != NULL) 
		&& (filebase != NULL) 
		&& (strlen(filepath)<=(MAX_PATH+MAX_FILENAME)) )

	token1 = filepath;
	token2 = strpbrk(token1, "/\\");
	while( token2 )
	{
		token1 = token2+1;
		token2 = strpbrk(token1, "/\\");
	}
	if(token1 == filepath)
		strcpy(filebase, "./");
	else
	{	while(filepath[path_prefix] == '/' || filepath[path_prefix] == '\\' || filepath[path_prefix] == '.' )
			path_prefix++;
		BaseLen = strlen(filepath) - strlen(token1) - path_prefix;
		strncpy(filebase, filepath+path_prefix, BaseLen);
		filebase[BaseLen] = '\0';
	}
	
}


int CheckWriteContext( char *filepath, int mode)
{	int writeIt = 0;

	if( FileExist(filepath) )
	{
		char c = '\0';
		switch (mode)
		{	
		    case ALWAYS:
			writeIt = 1;
			break;
		    case NEVER:
			writeIt = 0;
			break;
		    case PROMPT:
			read:
			PRINT(("\nFile \"%s\" exists, overwrite? [y/n] ", filepath))
			fflush(stdin);
			scanf("%c", &c);
			if(c == 'y')
				writeIt = 1;
			else if(c == 'n')
				writeIt = 0;
			else
				goto read;
			break;
		    default:
			PRINT(("CheckWriteContext: ERROR, Unknown overwrite mode"))
			writeIt = 0;
			break;
		}
	}
	else
	{
		struct stat stats;
		char *DirName_buf;
		char *DirName;
		char *Prefix_buf;
		char *Prefix;

		if (!(DirName_buf = DirName = (char*)malloc(MAX_PATH)) ||
		    !(Prefix_buf = Prefix = (char*)malloc(MAX_PATH))) {
			PRINT(("CheckWriteContext: ERROR, No Memory"))
			exit(1);
		}
		GetFileBase(filepath, DirName);
		if(DirName[0] == '/' || DirName[0] == '\\')
			strcpy(Prefix, "\0");
		else
			strcpy(Prefix, ".\0");

		DirName = strtok( DirName, "/\\");
		while( DirName )
		{
			strcat(Prefix, "/");
			strcat(Prefix, DirName);
			if(stat(Prefix, &stats) != -1)
			{
				if( !(stats.st_mode & S_IFDIR ))
				{
					PRINT(("Unable to create directory %s, because a file with the same name already exists\n", Prefix))
					goto end;
				}
			}
			else
			{
				PRINT(("Creating directory %s\n", Prefix))
#ifdef WIN32
				if(mkdir( Prefix ) <0 )
#else
				if(mkdir( Prefix, 0755) <0 )
#endif
				{
					EXIT(("%s : Unable to create directory\n", Prefix))
				}
			}
			DirName = strtok( NULL, "/\\");
		} 
		free(DirName_buf);
		free(Prefix_buf);
		writeIt = 1;
	}
	
end:	return writeIt;
}




void FFileInsert(ppFFile p_filelist, FFile newfile )
{
	pFFile List = *p_filelist;

	if (List == NULL) {
		if (!(*p_filelist = (FFile *)malloc(sizeof(FFile)))) {
			EXIT(("Error: Cannot alloc memory!\n"))
		}
		List = *p_filelist;
	}	
	else
	{
		while (List->next != NULL) {
			List = List->next;
		}
		if (!(List->next = (FFile *)malloc(sizeof(FFile)))) {
			EXIT(("Error: Cannot alloc memory!\n"))
		}
		List = List->next;
	}

	strncpy((List)->fullname, newfile.fullname, MAX_PATH+MAX_FILENAME );
	List->writeIt = newfile.writeIt;
	List->fd = newfile.fd;
	List->filesize = newfile.filesize;
	List->nbFragTot = newfile.nbFragTot;
	List->nbFragRcvd = newfile.nbFragRcvd;
	List->next = NULL;
}



pFFile FFileFind( const char *fullname, pFFile filelist )
{
	pFFile found = NULL;
	pFFile listloop = filelist;
	
	while(listloop != NULL)
	{
		if( !strcmp(listloop->fullname, fullname) ) {
			found = listloop;
			break;
		}
		listloop = listloop->next;
	}
	return found;
}


void FFileRemove ( const char *fullname, ppFFile p_filelist )
{
	pFFile found = FFileFind( fullname, *p_filelist );
	pFFile listloop = *p_filelist;

	if (found != NULL && listloop != NULL)
	{
		if(listloop == found)
		{
			*p_filelist = listloop->next;
			free(listloop);
		}
		else
		{
			while(listloop->next != found)
			{
				listloop = listloop->next;
			}

			listloop->next = found->next;
			free(found);
		}
	}
	else
	{
		EXIT(("Error: FFile %s not found!\n", fullname))
	}
	
}

void FFilePrintList(pFFile FFList)
{
	pFFile listloop = FFList;
	
	PRINT(("File fragments received:\n"))
	while(listloop != NULL)
	{
		PRINT(("\t%s (%d/%d)\n", listloop->fullname, listloop->nbFragRcvd, listloop->nbFragTot))
		listloop = listloop->next;
	}
	PRINT(("\n"))
}

