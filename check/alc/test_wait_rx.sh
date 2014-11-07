#!/bin/sh
#
# $Id: test_wait_rx.sh,v 1.2 2004/12/21 14:43:48 roca Exp $
#
#  Copyright (c) 1999-2004 INRIA - All rights reserved
#  (main authors: Vincent Roca - vincent.roca@inrialpes.fr
#                 Julien Laboure - julien.laboure@inrialpes.fr
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


#
# test the wait_event at the receiver
#


#
# scenario 1:
# The source tx a single ADU, waits the end (wait event),
# then issues a close.
# The receiver receives this ADU, waits the end (wait event),
# then issues a close.
#

recv_val=1
send_val=1

./test_wait_rx_sender &
recv_pid=$!
./test_wait_rx_receiver &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??


if [ ${send_val} -ne 0 ]
then
	echo "wait_rx Send Failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "wait_rx Recv Failed"
	exit 1
fi

