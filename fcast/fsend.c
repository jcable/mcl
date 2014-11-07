/* $Id: fsend.c,v 1.19 2005/01/11 13:12:22 roca Exp $ */
/*
 *  Copyright (c) 1999-2002 INRIA - Universite Paris 6 - All rights reserved
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
 * fsend.c
 *
 *	fcast sender side functions
 */
#include "fcast.h"


/*
 * Local variables
 */

/* available FEC codecs and associated information */
typedef struct fec_codec_info {
	int	available;		/* boolean */
	int	max_block_size;		/* raw value as defined by MCL */
	int	max_fragment_size;	/* big files are fragmented into */
					/* fragments of this size */
} fec_codec_info_t;

fec_codec_info_t	NULL_FEC_info;
fec_codec_info_t	RSE_FEC_info;
fec_codec_info_t	LDGM_STAIRCASE_FEC_info;
fec_codec_info_t	LDGM_TRIANGLE_FEC_info;



void FcastSend (void)
{
	int	Bytes_sent = 0;
#if defined(ALC)
	int	mcl_option;
#endif /* RM_PROTOCOL */

	/* Determine available FEC codes and init. associated info struct */
	FindAvailableFEC();
	/* Determine the appropriate modes */
#if defined(ALC)
	/*
	 * NB: always use LCT1 now, in all cases...
	 */
	mcl_option = MCL_SCHED_LCT1;
	if (mcl_ctl(id, MCL_OPT_SCHED, (void*)&mcl_option, sizeof(mcl_option))) {
		EXIT(("Fcast: mcl_ctl MCL_OPT_SCHED failed for LCT1\n"))
	}
	if (optimode == OPTIMIZE_SPACE) {
		mcl_option = MCL_SCHED_PARTIALLY_MIXED_ORDER;
		if (mcl_ctl(id, MCL_OPT_OBJ_SCHED, (void*)&mcl_option,
				sizeof(mcl_option)))
			EXIT(("Fcast: mcl_ctl MCL_OPT_SCHED failed\n"))
	} else if (optimode == OPTIMIZE_SPEED) {
		mcl_option = MCL_SCHED_MIXED_ORDER;
		if (mcl_ctl(id, MCL_OPT_OBJ_SCHED, (void*)&mcl_option, sizeof(mcl_option)))
			EXIT(("Fcast: mcl_ctl MCL_OPT_OBJ_SCHED failed\n"))
	} else if (optimode == OPTIMIZE_CPU) {
		mcl_option = MCL_SCHED_MIXED_ORDER;
		if (mcl_ctl(id, MCL_OPT_OBJ_SCHED, (void*)&mcl_option, sizeof(mcl_option)))
			EXIT(("Fcast: mcl_ctl MCL_OPT_OBJ_SCHED failed\n"))
	} else {
		EXIT(("Fcast: ERROR, invalid optimization mode!"))
	}
#elif defined(NORM)
	/* no equivalent */
#endif /* RM_PROTOCOL */

	/*
	mcl_option = 1;
	mcl_ctl(id, MCL_OPT_REUSE_APPLI_TX_BUFFER, (void*)&mcl_option,
		sizeof(mcl_option));
	*/
	/*
	 * Now send the file(s)
	 */
#if defined(ALC)
	if(mcl_ctl(id, MCL_OPT_KEEP_DATA, NULL, 0))
		EXIT(("Fcast: mcl_ctl KEEP_DATA failed\n"))
	if (recursive) {
		if (meta_object_mode) {
			if (mcl_ctl(id, MCL_OPT_KEEP_META_OBJECT, NULL, 0))
				EXIT(("Fcast: mcl_ctl KEEP_META_OBJECT failed\n"))
		}
		Bytes_sent = RecursiveSend(fileparam);
		if (meta_object_mode) {
			fec_codec_info_t *codec_info;
			/*
			 * If the meta_object mode is set, then select the
			 * FEC codec based on the total aggregate length...
			 * This is this call that will determine the effective
			 * codec used, no matter what may have been issued
			 * before. Usually the codec will be LDGM_XXX since
			 * the total aggregated length is usually larger than
			 * the maximum RSE block size threshold.
			 */
			codec_info = ChooseFEC(Bytes_sent);
			if (mcl_ctl(id, MCL_OPT_PUSH_META_OBJECT, NULL, 0))
				EXIT(("Fcast: mcl_ctl PUSH_META_OBJECT failed\n"))
		}
	} else {
		Bytes_sent = SendThisFile(fileparam);
	}
	if (mcl_ctl(id, MCL_OPT_PUSH_DATA, NULL, 0))
		EXIT(("Fcast: mcl_ctl PUSH_DATA failed\n"))
#elif defined(NORM)
	if (recursive) {
		Bytes_sent = RecursiveSend(fileparam);
	} else {
		Bytes_sent = SendThisFile(fileparam);
	}
#endif /* RM_PROTOCOL */
	/*
	 * Finished, close the MCL session.
	 */ 
	mcl_close(id);
	PRINT(("\nFcastSend completed. %d bytes sent\n", Bytes_sent))
}


#ifdef WIN32	/* directory parsing is OS dependant */


int RecursiveSend (char* Path)
{

	HANDLE		dirp = NULL;
	WIN32_FIND_DATA	entry;
	unsigned int	total_sent = 0;
	char		FullName[MAX_PATH + MAX_FILENAME];
	char		FindString[MAX_PATH];

	strcpy(FindString, Path);
	strcat(FindString, "\\*");
	
	dirp = FindFirstFile(FindString, &entry);

	if (dirp == INVALID_HANDLE_VALUE) {
		EXIT(("Fcast: ERROR, in recursive mode, the given parameter MUST BE a valid directory name\nAborting...\n"))
	}

	if (!IsDirDots (entry.cFileName)) {
		strcpy(FullName, Path);
		if(FullName[strlen(FullName)-1] == '\\')
			FullName[strlen(FullName)-1] = '/';

		if(FullName[strlen(FullName)-1] != '/')
			strcat(FullName,"/");

		strcat(FullName, entry.cFileName);

		if( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			ASSERT((strcmp(Path,FullName)))
			PRINT(("Entering Directory %s\n", FullName))
			total_sent+= RecursiveSend (FullName);
		} else {
			PRINT(("\nSending File %s\n", FullName))
			total_sent += SendThisFile (FullName);
		}
	}

	while ( FindNextFile(dirp, &entry) ) {
		if (IsDirDots (entry.cFileName))
			continue;

		strcpy(FullName, Path);
		if(FullName[strlen(FullName)-1] == '\\')
			FullName[strlen(FullName)-1] = '/';

		if(FullName[strlen(FullName)-1] != '/')
			strcat(FullName,"/");

		strcat(FullName, entry.cFileName);

		if( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			ASSERT((strcmp(Path,FullName)))
			PRINT(("Entering Directory %s\n", FullName))
			total_sent+= RecursiveSend (FullName);
		} else {
			PRINT(("\nSending File %s\n", FullName))
			total_sent += SendThisFile(FullName);
		}
	}
	FindClose(dirp);
	return total_sent;
}


#else  /* UNIX case */


int RecursiveSend (char* Path)
{
	unsigned int	total_sent = 0;
	struct dirent	*entry;
	struct stat	stats;
	char		FullName[MAX_PATH + MAX_FILENAME];
	DIR		*dirp;

	if ((dirp = opendir (Path)) == NULL) {
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
			total_sent+= RecursiveSend (FullName);
		} else {
			PRINT(("Sending File %s\n", FullName))
			total_sent += SendThisFile (FullName);
		}
	}
	closedir (dirp);
	return total_sent;
}


#endif /* OS */


int SendThisFile (char *file_path) //int fragment_len)
{
	fec_codec_info_t *codec_info;
	int		fragment_size;
	int		last_but_one_fragment_size;
	int		last_fragment_size;
	char 		file_base[MAX_PATH];
	char 		file_name[MAX_FILENAME];
	char		length[20];
	char		BlockTmp4[4];
	trailer_t	trailer;
	struct stat	file_stats;
	FILE		*file_to_send 	= NULL;
	char		*buf_file	= NULL;
	int		sent		= 0;
	int 		filesize	= 0;
	unsigned short	global_sum 	= 0;
	unsigned short	checksum	= 0;
	unsigned char	must_swap	= 0;
	int		nb_fragments	= 0;
	int		ObjectLength	= 0;
	int		CurrOffset	= 0;
	int		fragsize	= 0;
	int		i;
	char		fragment[20];
	char		offset[20];

	trailer.size = 0;
	trailer.buffer = NULL;

	if(FileExist(file_path)) {
		if(stat(file_path, &file_stats) == -1) {
			EXIT(("Fcast: SendThisFile: Error: stat()\n"))
			//PRINT(("Fcast: SendThisFile: Error: stat()\n"))
			//goto end;
		}
		file_to_send = fopen(file_path, "rb");
	} else {
		PRINT(("Fcast: SendThisFile: Error: %s, no such file!\n", file_path))
		goto end;
	}
	if (!(file_stats.st_mode & S_IFREG))
		EXIT(("Fcast: SendThisFile: Error: %s is not a regular file\n",
			file_path))		

	codec_info = ChooseFEC((int)(file_stats.st_size));
	fragment_size = codec_info->max_fragment_size;
	ASSERT(fragment_size > 0);

	if (file_stats.st_size < fragment_size) {
		nb_fragments = 1;
		last_but_one_fragment_size = -1;	/* avoid warnings */
	} else {
		nb_fragments = file_stats.st_size / fragment_size;
		last_fragment_size = file_stats.st_size % fragment_size;
		if (last_fragment_size > 0)
			nb_fragments++;
		/*
		 * if last fragment is less than half the fragment_size, then
		 * reduce size of last two fragments.
		 */
		if ((nb_fragments > 1) &&
		    (last_fragment_size < (fragment_size >> 1))) {
			int	sz;	/* size of last two fragments */
			sz = fragment_size + last_fragment_size;
			last_but_one_fragment_size = sz >> 1;
			last_fragment_size = sz - last_but_one_fragment_size;
		} else {
			last_but_one_fragment_size = fragment_size;
		}
		ASSERT(last_fragment_size >= 0);
		ASSERT(last_but_one_fragment_size >= 0);
		ASSERT(last_fragment_size <= fragment_size);
		ASSERT(last_but_one_fragment_size <= fragment_size);
	}
	PRINT(("*** Stripping file %s into %d fragment(s)\n",
		file_path, nb_fragments))

	GetFileBaseWithoutPrefix(file_path, file_base);

	for (i = 1; i <= nb_fragments; i++) {
		if (i == nb_fragments - 1) {
			/* last but one fragment may be shorter... */
			fragment_size = last_but_one_fragment_size;
		}
		/* make room in buf_file for additional trailer and checksum */
		if (!(buf_file = (char*)malloc(fragment_size + MAX_TRAILER_SIZE))) {
			EXIT(("Fcast: SendThisFile: Error: Cannot alloc memory!\n"))
		}

		if ((ObjectLength = fread(buf_file, 1, fragment_size ,
						file_to_send)) < 0)
			EXIT(("Fcast: SendThisFile: Error FcastSend: fread failed, returned %d\n", ObjectLength))

		if (strlen(file_base)>0) {
			AddMetadata(&trailer, "Content-Base", file_base);
		}

		GetFileName(file_path, file_name);
		AddMetadata(&trailer, "Content-Location", file_name);

		filesize = file_stats.st_size;
		sprintf(length, "%d", filesize);
		AddMetadata(&trailer, "Content-Filesize", length);

		fragsize = ObjectLength;
		sprintf(length, "%d", fragsize);
		AddMetadata(&trailer, "Content-Length", length);

		sprintf( fragment, "%d/%d", i, nb_fragments);
		AddMetadata(&trailer, "Content-Fragment", fragment);

		sprintf( offset, "%d", CurrOffset);
		CurrOffset += ObjectLength;
		AddMetadata(&trailer, "Content-Offset", offset);

		EndTrailer(&trailer);

		if(trailer.size + 4 > MAX_TRAILER_SIZE) {
			EXIT(("Fcast: SendThisFile: Error: Trailer is too big! (max=%d)\n", MAX_TRAILER_SIZE-4))
		}
		/*
		else	PRINT(("Trailer length is %ld bytes\n",trailer.size-4))
		*/
#if 0
		if (!(buf_file = (char*)realloc(buf_file, ObjectLength + trailer.size + 4))) {
			EXIT(("SendThisFile: Error: Cannot realloc memory!\n"))
		}
#endif
		memcpy(buf_file + ObjectLength, trailer.buffer, trailer.size);
		ObjectLength+= trailer.size;
		DeleteTrailer(&trailer);

		global_sum = ComputeSum((unsigned short*)buf_file,
					ObjectLength, &must_swap);
		checksum = ComputeChecksum(global_sum, must_swap);

		*(unsigned short*)BlockTmp4 = 0x0000;
		*(unsigned short*)(BlockTmp4+2) = checksum;

		memcpy(buf_file + ObjectLength, BlockTmp4, 4);
		ObjectLength += 4;

		PRINT(("Sending file %s slice %d (Data: %d Bytes - Trailer: %d Bytes)\n", file_path, i, fragsize, ObjectLength-fragsize-8))
		if (mcl_send(id, buf_file, ObjectLength) < 0)
			EXIT(("Fcast: mcl_send %ld failed\n", trailer.size))
		sent+= ObjectLength;

		if(buf_file)
			free(buf_file);
	}

	fclose(file_to_send);
//	ASSERT(sent == file_stats.st_size)

	PRINT(("File %s (%d Bytes) sent.\n\t%d fragments (objects) transfered for this file.\n\t%d Bytes transfered for this file.\n", file_path, filesize, nb_fragments, sent))

end:
	return sent;
}


/*
 * Updates the FEC available global variables depending on the available
 * FEC codecs.
 * Sets the FEC ratio for each possible FEC codec.
 * Retrieves the maximum block size for each possible FEC codec.
 */
void
FindAvailableFEC (void)
{
	int	codec;	/* temp variable */
	int	max_sz;	/* temp variable */

	ASSERT(fec_ratio >= 1.0);
	/*
	 * NULL_FEC codec first (should always be supported)
	 */
	codec = MCL_FEC_SCHEME_NULL;
	if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec, sizeof(codec))) {
		EXIT(("Fcast: ERROR, FEC codec NULL not available\n"))
	} else {
		NULL_FEC_info.available = 1;
		/*
		 * determine the max block size, as defined by MCL,
		 * and the associated max file fragment size, used
		 * by FCAST.
		 */
		if (mcl_ctl(id,
			    MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC,
			    (void*)&max_sz, sizeof(max_sz))) {
			EXIT(("Fcast: ERROR, mcl_ctl failed for MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC for LDGM codec\n"))
		}
		NULL_FEC_info.max_block_size = max_sz;
		NULL_FEC_info.max_fragment_size =
						min(NULL_FEC_MAX_FRAGMENT_SIZE,
						    max_sz - MAX_TRAILER_SIZE);
	}
	/*
	 * now let's see the other FEC codecs
	 */
	if (fec_ratio == 1.0) {
		/* user wants no FEC */
		RSE_FEC_info.available = 0;
		LDGM_STAIRCASE_FEC_info.available = 0;
		LDGM_TRIANGLE_FEC_info.available = 0;
	} else {
		/* user wants some FEC */
		/*
		 * RSE
		 */
		codec = MCL_FEC_SCHEME_RSE_129_0;
		if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec,
				sizeof(codec))) {
			RSE_FEC_info.available = 0;
		} else {
			RSE_FEC_info.available = 1;
			if (mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio,
					sizeof(fec_ratio))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for FEC_RATIO %.3f and RSE\n", fec_ratio))
			}
			/*
			 * determine the max block size, as defined by MCL,
			 * and the associated max file fragment size, used
			 * by FCAST.
			 */
			if (mcl_ctl(id,
				    MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC,
				    (void*)&max_sz, sizeof(max_sz))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC for RSE codec\n"))
			}
			RSE_FEC_info.max_block_size = max_sz;
			RSE_FEC_info.max_fragment_size =
						min(RSE_MAX_FRAGMENT_SIZE,
						    max_sz - MAX_TRAILER_SIZE);
			/* tests MCL blocking algo: define a large frag size */
			//RSE_FEC_info.max_fragment_size = 512*1024;
		}
#if defined(ALC)
		/*
		 * LDGM STAIRCASE
		 */
		codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
		if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec,
				sizeof(codec))) {
			LDGM_STAIRCASE_FEC_info.available = 0;
		}else {
			LDGM_STAIRCASE_FEC_info.available = 1;
			if (mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio,
					sizeof(fec_ratio))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for FEC_RATIO %f for LDGM STAIRCASE codec\n", fec_ratio))
			}
			/*
			 * determine the max block size, as defined by MCL,
			 * and the associated max file fragment size, used
			 * by FCAST.
			 */
			if (mcl_ctl(id,
				    MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC,
				    (void*)&max_sz, sizeof(max_sz))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC for LDGM STAIRCASE codec\n"))
			}
			LDGM_STAIRCASE_FEC_info.max_block_size = max_sz;
			LDGM_STAIRCASE_FEC_info.max_fragment_size =
						min(LDGM_MAX_FRAGMENT_SIZE,
						    max_sz - MAX_TRAILER_SIZE);
		}
		/*
		 * LDGM TRIANGLE
		 */
		codec = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
		if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec,
				sizeof(codec))) {
			LDGM_TRIANGLE_FEC_info.available = 0;
		}else {
			LDGM_TRIANGLE_FEC_info.available = 1;
			if (mcl_ctl(id, MCL_OPT_FEC_RATIO, (void*)&fec_ratio,
					sizeof(fec_ratio))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for FEC_RATIO %f for LDGM TRIANGLE codec\n", fec_ratio))
			}
			/*
			 * determine the max block size, as defined by MCL,
			 * and the associated max file fragment size, used
			 * by FCAST.
			 */
			if (mcl_ctl(id,
				    MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC,
				    (void*)&max_sz, sizeof(max_sz))) {
				EXIT(("Fcast: ERROR, mcl_ctl failed for MCL_OPT_GET_MAX_BLOCK_SIZE_FOR_CURRENT_FEC for LDGM TRIANGLE codec\n"))
			}
			LDGM_TRIANGLE_FEC_info.max_block_size = max_sz;
			LDGM_TRIANGLE_FEC_info.max_fragment_size =
						min(LDGM_MAX_FRAGMENT_SIZE,
						    max_sz - MAX_TRAILER_SIZE);
		}
#elif defined(NORM)
		LDGM_STAIRCASE_FEC_info.available = 0;
		LDGM_TRIANGLE_FEC_info.available = 0;
#endif /* RM_PROTOCOL */
		// Remove following two line comment to test with RSE
		//LDGM_STAIRCASE_FEC_info.available = 0;
		//LDGM_TRIANGLE_FEC_info.available = 0;
		if (!RSE_FEC_info.available && !LDGM_STAIRCASE_FEC_info.available && !LDGM_TRIANGLE_FEC_info.available) {
			EXIT(("Fcast: ERROR, at least one of RSE or LDGM codecs must be available\n"))
		}
	}
}


/*
 * Choose the most appropriate FEC codec, depending on the file size
 * and the available FEC codecs.
 * @param file_size	file size
 * @return		FEC codec chosen
 */
fec_codec_info_t *
ChooseFEC (int	file_size)
{
	int		codec;
	fec_codec_info_t *codec_info;

	if (fec_ratio == 1.0) {
		ASSERT(NULL_FEC_info.available);
		codec = MCL_FEC_SCHEME_NULL;
		codec_info = &NULL_FEC_info;
		goto codec_found;
	}
#if defined(ALC)
	/*
	 * With ALC, use LDGM first for big files and RSE for
	 * small files, and if the default choice is not available,
	 * use the other one.
	 */
	if (file_size >= RSE_LDGM_FILE_SIZE_THRESHOLD) {
		if (fec_ratio >= 2.5 && LDGM_STAIRCASE_FEC_info.available) {
			/*
			 * Use LDGM Staircase with large FEC expansion ratios.
			 */
			codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
			codec_info = &LDGM_STAIRCASE_FEC_info;
		} else if (LDGM_TRIANGLE_FEC_info.available) {
			/*
			 * Use LDGM Triangle with small FEC expansion ratios.
			 */
			codec = MCL_FEC_SCHEME_LDGM_TRIANGLE_132_1;
			codec_info = &LDGM_TRIANGLE_FEC_info;
		} else {
			ASSERT(RSE_FEC_info.available)
			codec = MCL_FEC_SCHEME_RSE_129_0;
			codec_info = &RSE_FEC_info;
		}
	} else {
		if (RSE_FEC_info.available) {
			codec = MCL_FEC_SCHEME_RSE_129_0;
			codec_info = &RSE_FEC_info;
		} else {
			ASSERT(LDGM_STAIRCASE_FEC_info.available)
			codec = MCL_FEC_SCHEME_LDGM_STAIRCASE_132_0;
			codec_info = &LDGM_STAIRCASE_FEC_info;
		}
	}
#elif defined(NORM)
	/* With NORM, use RSE (LDGM not supported) */
	ASSERT(RSE_FEC_info.available);
	codec = MCL_FEC_SCHEME_RSE_129_0;
	codec_info = &RSE_FEC_info;
#endif /* RM_PROTOCOL */

codec_found:
	/* set the codec now */
	if (mcl_ctl(id, MCL_OPT_SET_FEC_CODE, (void*)&codec, sizeof(codec))) {
		EXIT(("Fcast: ERROR, ctl for MCL_OPT_SET_FEC_CODE (%d) failed\n", codec))
	}
	return codec_info;
}


#if 0
/*
 * Determines the optimum file segmentation, and the appropriate FEC codec
 * to use.
 * @param
 * @return
 */
int
FileSegmentation
{
}
#endif
