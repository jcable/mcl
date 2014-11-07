#!/bin/sh
#
# $Id: test_dyn_tsi.sh,v 1.2 2004/12/21 14:43:45 roca Exp $
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


# Principle:
#	The source uses TSI==1 to announce the new TSI in the first object,
#	then changes to this new TSI, and sends a second object.
#	The receiver starts with TSI==1, retrieves the new TSI, changes to
#	this TSI dynamically, and receives the second object.

#./test_dyn_tsi -recv > /tmp/test_dyn_tsi_rx.trc 2>&1 &
./test_dyn_tsi -recv &
recv_pid=$!

#./test_dyn_tsi -send > /tmp/test_dyn_tsi_tx.trc 2>&1 &
./test_dyn_tsi -send &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "test DYNAMIC TSI Send Failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "test DYNAMIC TSI Recv Failed"
	exit 1
fi
