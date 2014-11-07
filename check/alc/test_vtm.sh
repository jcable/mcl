#!/bin/sh
#
# $Id: test_vtm.sh,v 1.2 2004/12/21 14:43:48 roca Exp $
#
#  Copyright (c) 1999-2004 INRIA - All rights reserved
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


# test the MCL with an application that uses two sessions
# each of them using the Virtual Tx Memory service
# An object is sent on each session, containing the session value.


# skipped !
#echo "WARNING: test skipped !!!!!!"
#exit 0


#./test_vtm_rx > /tmp/mcl_vtm_rx.trc 2>&1 &
./test_vtm_rx &
recv_pid=$!

#./test_vtm_tx > /tmp/mcl_vtm_tx.trc 2>&1 &
./test_vtm_tx &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "VTM (Virtual Tx Memory service) send failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "VTM (Virtual Tx Memory service) recv failed"
	exit 1
fi
