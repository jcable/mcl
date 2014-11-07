#!/bin/sh
#
# $Id: test_several_fec_codecs.sh,v 1.3 2004/12/21 14:43:48 roca Exp $
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
#	The source uses several FEC codecs, one per object.
#	The receivers decodes each of them using the appropriate codec.

#./test_several_fec_codecs -recv > /tmp/test_several_fec_codecs_rx.trc 2>&1 &
./test_several_fec_codecs -recv &
recv_pid=$!

#./test_several_fec_codecs -send > /tmp/test_several_fec_codecs_tx.trc 2>&1 &
./test_several_fec_codecs -send &
send_pid=$!

wait ${recv_pid}
recv_val=$?

wait ${send_pid}
send_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "test several FEC codecs (sender) failed"
	exit 1
elif [ ${recv_val} -ne 0 ]
then
	echo "test several FEC codecs (receiver) failed"
	exit 1
fi
