/* $Id: file_tools.cpp,v 1.2 2005/05/12 16:03:23 moi Exp $ */
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

#include "macros.h"

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
