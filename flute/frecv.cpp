/* $Id: frecv.cpp,v 1.12 2005/05/12 16:03:30 moi Exp $ */
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
 * frecv.c
 *
 *	flute receiver side functions
 */
#include "../src/flute_lib/FluteAPI.h"
#include "frecv.h"
#include "macros.h"

extern class FluteReceiver	*myflutereceiver;
extern bool interactive;

void FluteRecv(void)
{
	UINT64	Bytes_received = 0;

	myflutereceiver->setSelectAll (!interactive);

	PRINT(("Waiting for data...\n"))
	Bytes_received = myflutereceiver->recv(true);
	PRINT(("\nFluteRecv completed, %llu bytes received\n", Bytes_received))
}

