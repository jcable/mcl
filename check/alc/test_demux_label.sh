#!/bin/sh
#
# $Id: test_demux_label.sh,v 1.3 2004/12/21 14:43:45 roca Exp $
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


# test the MCL with an application that uses different sessions
# with different demux labels.
# An object is sent on each session, containing the demux label value.


echo ""
echo "** TSI tests with small TSI identifiers**"
echo ""

#./test_demux_label1_rx > /tmp/mcl_demux_rx.trc 2>&1 &
./test_demux_label1_rx &
recv_pid=$!

sleep 1;	# avoid synchro problems

#./test_demux_label1_tx > /tmp/mcl_demux_tx.trc 2>&1 &
./test_demux_label1_tx &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "DEMUX_LABEL Send Failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "DEMUX_LABEL Recv Failed"
	exit 1
fi

echo ""
echo ""
echo "** TSI tests with large TSI identifiers**"
echo ""

#./test_demux_label2_rx > /tmp/mcl_demux_rx.trc 2>&1 &
./test_demux_label2_rx &
recv_pid=$!

sleep 1;	# avoid synchro problems

#./test_demux_label2_tx > /tmp/mcl_demux_tx.trc 2>&1 &
./test_demux_label2_tx &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "DEMUX_LABEL Send Failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "DEMUX_LABEL Recv Failed"
	exit 1
fi
