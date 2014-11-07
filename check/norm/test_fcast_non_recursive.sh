#!/bin/sh
#
# $Id: test_fcast_non_recursive.sh,v 1.2 2004/09/22 14:10:20 chneuman Exp $
#
#  Copyright (c) 1999-2003 INRIA - Universite Paris 6 - All rights reserved
#  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
#                 Vincent Roca - vincent.roca@inrialpes.fr
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
# fcast validation script #2 : non recursive Send/Recv Test
#
#
host_name=`uname -s`


case ${host_name} in

	Linux)
	echo "Running FCAST Linux non recursive send/recv test"
	fcast_path="../../bin/linux/fcastn"
	;;

	SunOS)
	echo "Running FCAST Solaris non recursive send/recv test"
	fcast_path="../../bin/solaris/fcastn"
	;;

	FreeBSD)
	echo "Running FCAST FreeBSD non recursive send/recv test"
	fcast_path="../../bin/freebsd/fcastn"
	;;

	# other OS???? todo
esac

# for debug...
#verbosity_recv='-v1'		# receiver part
verbosity_recv=''
#verbosity_send='-v5'		# sender part
verbosity_send=''

# file sent
file='./Other_Files/i-am-a-BIG.file'


#
# multicast tests first
#
echo ""
echo "** Multicast tests..."
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -never -a225.3.4.5/9998 &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -a225.3.4.5/9998 ${file} &
fcast_send_pid=$!

wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??

cd ../..
diff ./fcast_test/send/${file} ./fcast_test/recv/${file}
diff_val=$?


if [ ${send_val} -ne 0 ]
then
	echo "FCAST Send Failed"
	exit 1

elif [ ${recv_val} -ne 0 ]
then
	echo "FCAST Recv Failed"
	exit 1

elif [ ${diff_val} -ne 0 ]
then
	echo "Test failed: received files do not match sent files!"
	exit 1
fi


#
# and unicast tests...
#
echo ""
echo "** And unicast tests now..."
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -never -a127.0.0.1/9998 &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -a127.0.0.1/9998 ${file} &
fcast_send_pid=$!

wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??

cd ../..
diff ./fcast_test/send/${file} ./fcast_test/recv/${file}
diff_val=$?


# do not test send_val return value here as in unicast,
# the sender is always stopped as soon as the receiver leaves
# with a sendmsg error. This is normal so ignore it and only
# rely on diff tests
#
if [ ${recv_val} -ne 0 ]
then
	echo "FCAST Recv Failed"
	exit 1

elif [ ${diff_val} -ne 0 ]
then
	echo "Test failed: received files do not match sent files!"
	exit 1

else
	exit 0
fi
