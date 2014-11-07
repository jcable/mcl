/* $Id: test_open_close.cpp,v 1.2 2005/05/12 16:03:17 moi Exp $ */
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
 
 
/*
 * test_open_close.c
 *
 *	open and closes a flute session.
 */
 
#include "../../src/flute_lib/FluteAPI.h"


/* the flute session class */
class Flute	*myflute = NULL;
class FluteSender	*myflutesender = NULL;
class FluteReceiver	*myflutereceiver = NULL;


int main (int argc, char *argv[])
{

	myflute = new Flute();
	myflutesender = new FluteSender();
	myflutereceiver = new FluteReceiver();
	
	
	delete myflute;
	delete myflutesender;
	delete myflutereceiver;
	
}
