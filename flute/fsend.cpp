/* $Id: fsend.cpp,v 1.13 2005/05/12 16:03:30 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *		  Julien Laboure - julien.laboure@inrialpes.fr
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
 * fsend.c
 *
 *	flute sender side functions
 */
#include "../src/flute_lib/FluteAPI.h"
#include "fsend.h"
#include "file_tools.h"
#include "macros.h"

extern char	**fileparam;
extern class FluteSender	*myflutesender;
extern int 	nb_tx;


void FluteSend (void)
{
	UINT64	Bytes_sent = 0;
	
	int i = 0;
	while(fileparam[i] != NULL && i < MAXNUMBER_OF_FILES)
	{
#ifdef WIN32
		WIN32_FIND_DATA entry;
		FindFirstFile(fileparam[i], &entry);
		if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
		struct stat stats;
		stat(fileparam[i], &stats);
		if (S_ISDIR(stats.st_mode))
#endif
		{
			RecursiveSend(fileparam[i]);
		}
		else {
			SendThisFile(fileparam[i]);
		}
		i++;
	}
	
	Bytes_sent = myflutesender->carousel->startTxCycles(nb_tx,true);
	
	/* free list of files */
	i = 0;
	while(fileparam[i] != NULL && i < MAXNUMBER_OF_FILES) {
		free(fileparam[i]);
		i++;
	}
	free(fileparam);
	
	PRINT(("\nFluteSend complete. %llu bytes sent\n", Bytes_sent))
}


#ifdef WIN32

void RecursiveSend (char* Path)
{

	HANDLE			dirp = NULL;
	WIN32_FIND_DATA entry;

	char FullName[MAX_PATH + MAX_FILENAME];
	char FindString[MAX_PATH];
	strcpy(FindString, Path);
	strcat(FindString, "\\*");
	
	dirp = FindFirstFile(FindString, &entry);

	if (dirp == INVALID_HANDLE_VALUE)
	{
		EXIT(("Flute: ERROR, in recursive mode, the given parameter MUST BE a valid directory name\nAborting...\n"))
	}

	if (!IsDirDots (entry.cFileName))
	{
		strcpy(FullName, Path);
		if(FullName[strlen(FullName)-1] == '\\')
			FullName[strlen(FullName)-1] = '/';

		if(FullName[strlen(FullName)-1] != '/')
			strcat(FullName,"/");

		strcat(FullName, entry.cFileName);

		if( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			ASSERT((strcmp(Path,FullName)));
			PRINT(("Entering Directory %s\n", FullName));
			RecursiveSend (FullName);
		}
		else
		{
			PRINT(("\nSending File %s\n", FullName))
			SendThisFile (FullName);
		}
	}

	while ( FindNextFile(dirp, &entry) )
	{
		if (IsDirDots (entry.cFileName))
			continue;

		strcpy(FullName, Path);
		if(FullName[strlen(FullName)-1] == '\\')
			FullName[strlen(FullName)-1] = '/';

		if(FullName[strlen(FullName)-1] != '/')
			strcat(FullName,"/");

		strcat(FullName, entry.cFileName);

		if( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			ASSERT((strcmp(Path,FullName)))
			PRINT(("Entering Directory %s\n", FullName))
			RecursiveSend (FullName);
		}
		else
		{
			PRINT(("\nSending File %s\n", FullName))
			SendThisFile (FullName);
		}
	}
	FindClose(dirp);
}


#else

void RecursiveSend (char* Path)
{
	struct dirent *entry;
	struct stat stats;
	char FullName[MAX_PATH + MAX_FILENAME];
	DIR *dirp;

	if ((dirp = opendir (Path)) == NULL)
	{
		perror(Path);
		EXIT(("RecursiveSend: ERROR, opendir failed\n"))
	}

	/*
	For SOLARIS users:
	Why doesn't readdir work? It chops the first two characters of
	all filenames.

	You're probably linking with libucb and didn't read question
	6.18. (Readdir in libucb.so wants you to include sys/dir.h,
	but many SunOS 4.1.x programs included <dirent.h>, consequently,
	you're mixing native <dirent.h> struct dirent with libucb
	readdir(). The symptom of this mixup is that the first two
	characters of each filename are missing. Make sure you use a
	native compiler (default /opt/SUNWspro/bin/cc, which may not be in
	your PATH), and not /usr/ucb/cc. 
	*/

	while ((entry = readdir(dirp)) != NULL) {
		if (IsDirDots (entry->d_name))
			continue;

		strcpy(FullName, Path);
		if (FullName[strlen(FullName)-1] != '/')
			strcat(FullName, "/");

		strcat(FullName, entry->d_name);

		if (stat(FullName, &stats) == -1) {
			perror("RecursiveSend: ERROR, stat failed");
			PRINT(("RecursiveSend: FullName=%s\nentry->d_name=%s\nPath=%s\n", FullName, entry->d_name, Path))
			EXIT(("RecursiveSend: ERROR, stat() failed\n"))
		}

		if (S_ISDIR(stats.st_mode)) {
			ASSERT((strcmp(Path,FullName)))
			PRINT(("Entering Directory %s\n", FullName))
			RecursiveSend (FullName);
		} else {
			PRINT(("Sending File %s\n", FullName))
			SendThisFile (FullName);
		}
	}
	closedir (dirp);
}

#endif



TOI_t SendThisFile (char *file_path)
{

	struct		stat file_stats;
	int		file_to_send 	= 0;	
	TOI_t	 	toi = 0;

	if (FileExist(file_path)) {
		if (stat(file_path, &file_stats) == -1) {
			EXIT(("Flute: SendThisFile: Error: stat()\n"))
			//PRINT(("Flute: SendThisFile: Error: stat()\n"))
			//goto end;
		}
#ifdef WIN32
		file_to_send = open(file_path, O_RDONLY | O_BINARY);
#else
		file_to_send = open(file_path, O_RDONLY);
#endif
		if (file_to_send < 0) {
			EXIT(("Flute: open failed for file %s\n", file_path))
		}
	} else {
		PRINT(("Flute: SendThisFile: Error: %s, no such file!\n", file_path))
		close(file_to_send);
		return 0;
	}
	if (!(file_stats.st_mode & S_IFREG))
		EXIT(("Flute: SendThisFile: Error: %s is not a regular file\n", file_path))		


	close(file_to_send);
	
	myflutesender->carousel->addFile(file_path);
	
	return toi;
}


