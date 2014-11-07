/* $Id: display.cpp,v 1.9 2005/05/17 13:31:36 moi Exp $ */
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

#include "../src/flute_lib/FluteAPI.h"
#include "display.h"
#include "macros.h"

extern class Flute	*myflute;
extern bool 		interactive ;

#ifndef WIN32	
static struct termios initial_settings;

static void reset_term(void)
{
	tcsetattr(0, TCSANOW,  &initial_settings);
}
	
static void sig_catcher (int sig)
{
	reset_term();
}
#endif

#ifdef WIN32
void *display_callback(class FluteFileInfo* fileinfolist);
#endif

void *display(void * arg)
{

	int interval;
	interval = 1;	/* Default update rate is 1 seconds */

#ifndef WIN32
	struct termios new_settings;
	struct sigaction sa;
	fd_set readfds;
	unsigned char c;
	struct timeval tv;
	

	unsigned int highlight=0;
	TOI_t toi;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	tcgetattr(0, &initial_settings);
	memcpy(&new_settings, &initial_settings, sizeof(struct termios));
	new_settings.c_lflag &= ~(ISIG | ICANON); /* unbuffered input */
	
	/* Turn off echoing */		
	new_settings.c_lflag &= ~(ECHO | ECHONL);
	signal (SIGTERM, sig_catcher);
	sigaction (SIGTERM, (struct sigaction *) 0, &sa);
	sa.sa_flags |= SA_RESTART;
#if defined(FREEBSD) || defined(SOLARIS)
	sa.sa_flags &= ~SA_NOCLDSTOP;
#else
	sa.sa_flags &= ~SA_INTERRUPT;
#endif
	sigaction (SIGTERM, &sa, (struct sigaction *) 0);
	sigaction (SIGINT, &sa, (struct sigaction *) 0);
	tcsetattr(0, TCSANOW, &new_settings);
	atexit(reset_term);
	
	while(1)
	{

		tv.tv_sec = interval;
		tv.tv_usec = 0;
		
		toi = displayFiles(highlight);
		FD_ZERO (&readfds);
		FD_SET (0, &readfds);
		select (1, &readfds, NULL, NULL, &tv);
		if (FD_ISSET (0, &readfds)) {
			if (read (0, &c, 1) <= 0) {   /* signal */
				EXIT(("Keyboard input error\n"));
			}
			switch(c) {
			case 113: 
				myflute->abort();						
				EXIT(("Good Bye\n")); /**q**/
			 	break;
			case 27: /**arrow pad**/
				if (read (0, &c, 1) <= 0) {   /* signal */
					EXIT(("Keyboard input error\n"));
				}
				if (read (0, &c, 1) <= 0) {   /* signal */	
					EXIT(("Keyboard input error\n"));
				}
				if (c==66) highlight++;	
				if (c==65 && highlight!=0) highlight--;
				break;
			
			case 10: 	/**ENTER**/
				if (interactive != false) break;
				if (toi != 0 && myflute->isSender() == true) ((class FluteReceiver *)myflute)->selectTOI(toi);
				
				//else FFileRemoveTOI(toi,&myfiles); /deselection not supported yet
				break;
			}
		}
	}

#else /* WIN32 */

	((class FluteReceiver *)myflute)->setCallbackReceivedNewFileDescription(display_callback);


	while (1)
	{
		Sleep(interval*1000);
	}

#endif
	return NULL;
}


#ifndef WIN32
/**
 * Displays the FDT on stdout.
 * @param highlight	index of line highlighted in the File list.
 * @return		TOI of the highligted line.
 */
TOI_t displayFiles(unsigned int highlight)
{

	class FluteFileInfo* fileinfolist = NULL;
	class FluteFileInfo* fileinfo;
	TOI_t returntoi = 0;
	int j;

	fileinfolist = myflute->getFileInfoList();

	/*clear screen*/
	if (myflute->getVerbosity() == 0)	printf("\e[H\e[J");

	if (fileinfolist == NULL) {	
		printf("FDT is empty\n");
		goto exit;
	}

	printf("TOI\tSelect\tRcvd\tLength\tMD5sum\tContent-Location\n\n");

	/*
	 * Now go through all entries in the file list...
	 */
	fileinfo = fileinfolist;
	j = 0;
	while (fileinfo != NULL)
	{
	
		if (j == highlight)
			printf("\e[7m");	

		printf(" %lu\t", fileinfo->getTOI());

		if (fileinfo->isSelected() == true) {
			/* yes, this file is expected! */
			printf("X\t");
			double	val;	/* percentage */
			val = (double)fileinfo->getBytesRcvd() / (double)fileinfo->getTransferLength() * 100.0;
			printf("%.1f\%\t", val);
		} else {
			printf("\t");
		}

		if (fileinfo->getContentLength() == 0)
			printf("?\t");
		else if(fileinfo->getContentLength() < 0)
			printf("NOK\t");
		else 
			printf("%lu\t", fileinfo->getContentLength());

		if (fileinfo->getIntegrity() == 1)
			printf("OK\t");
		else if (fileinfo->getIntegrity() == -1)
			printf(" NOK \t");
		else
			printf("?\t");

		char* filename;
		int len;
		fileinfo->getFilename(&filename,&len);
		printf("%s ",filename);
		printf("\n");

		if (j == highlight) {
			printf("\e[0m");
			returntoi = fileinfo->getTOI();
		}

		j++;
		fileinfo = fileinfo->getNextFile();	
	}
		
exit:
	return returntoi;	       
}
#endif


#ifdef WIN32
void *display_callback(class FluteFileInfo* fileinfolist) {
	
	int j;
	class FluteFileInfo* fileinfo;

	if (fileinfolist == NULL) {	
		printf("FDT is empty\n");
		goto exit;
	}

	printf("TOI\tSelect\tRcvd\tLength\tMD5sum\tContent-Location\n\n");

	/*
	 * Now go through all entries in the file list...
	 */
	fileinfo = fileinfolist;
	j = 0;
	while (fileinfo != NULL)
	{
		printf(" %lu\t", fileinfo->getTOI());

		if (fileinfo->isSelected() == true) {
			/* yes, this file is expected! */
			printf("X\t");
			double	val;	/* percentage */
			val = (double)fileinfo->getBytesRcvd() / (double)fileinfo->getTransferLength() * 100.0;
			printf("%.1f\%\t", val);
		} else {
			printf("\t");
		}

		if (fileinfo->getContentLength() == 0)
			printf("?\t");
		else if(fileinfo->getContentLength() < 0)
			printf("NOK\t");
		else 
			printf("%lu\t", fileinfo->getContentLength());

		if (fileinfo->getIntegrity() == 1)
			printf("OK\t");
		else if (fileinfo->getIntegrity() == -1)
			printf(" NOK \t");
		else
			printf("?\t");

		char* filename;
		int len;
		fileinfo->getFilename(&filename,&len);
		printf("%s ",filename);
		printf("\n");

		j++;
		fileinfo = fileinfo->getNextFile();	
	}
		
exit:

	return NULL;
}
#endif

