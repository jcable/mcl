#!/bin/sh
#
# $Id: test_tx.sh,v 1.1.1.1 2003/09/03 12:45:41 chneuman Exp $
#
#  Copyright (c) 2003 INRIA - All rights reserved
#  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
#  USA.


# test the MCL with an application that only performs 
# mcl_open and immediately mcl_close

./test_tx 
val=$?

echo "${pid} returns ${val}"
if [ ${val} -ne 0 ] ; then
	exit 1
else
	exit 0
fi

