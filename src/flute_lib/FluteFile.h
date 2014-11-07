/* $Id: FluteFile.h,v 1.2 2005/05/12 16:03:36 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
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
 
#ifndef FLUTE_FILE_H
#define FLUTE_FILE_H

#ifdef WIN32
#include <direct.h>
#endif

class FluteFile : FluteFileInfo {

public:
	/****** Public Members ************************************************/
	/**
	 * Default constructor.
	 */
	FluteFile (class flute_cb * flutecb);

	/**
	 * Default destructor.
	 */
	~FluteFile ();

	class FluteFileInfo *createFileInfo();

#ifdef OPENSSL
	INT32 	FFilemd5sum_check();	
#endif
			
	/********* variables ****************/
	
	
	INT32		fd;		// File descriptor
	INT32		writeIt;	// Shall we write it or not?
					// In some cases it means overwritting
					// an existing file.
	unsigned char   md5sum[MD5BASE64_LENGTH];// MD5sum (in BASE64) indicated by FDT

	FluteFile * nextFile; // pointer to the next File (if any).

	friend class FluteFDT;
	friend class FluteFileList;
	friend class FluteReceiver;


	/******** static functions ***********/
	static int FileExist( const char *filepath)
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



	static int IsDirDots (const char *path)
	{
	  return (path[0] == '\0'
		  || (path[0] == '.' && (path[1] == '\0'
				      || (path[1] == '.' && path[2] == '\0'))));
	}

	static void GetFileBase( char *filepath, char *filebase )
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
	
	static int CheckWriteContext( class flute_cb *flutecb, char *filepath, int mode)
	{	
		int writeIt = 0;
	
		if( FluteFile::FileExist(filepath) )
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
						PRINT(("Unable to create directory %s, because a file with 	the same name already exists\n", Prefix))
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


	
private:
	/************* private members ******/
	class flute_cb* flutecb;
		
};

#endif
