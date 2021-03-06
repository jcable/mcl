#!/bin/sh
#
# $Id: test_fcast_xfert.sh,v 1.4 2004/12/21 14:43:47 roca Exp $
#
#  Copyright (c) 1999-2004 INRIA - All rights reserved
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


# skipped !
# echo "WARNING: test skipped !!!!!!"
# exit 0

#
# fcast validation script #1 : Send/Recv Test
#
#
host_name=`uname -s`


case ${host_name} in

	Linux)
	echo "Running FCAST Linux Send/Recv Test"
	fcast_path="../../bin/linux/fcast"
	;;

	SunOS)
	echo "Running FCAST Solaris Send/Recv Test"
	fcast_path="../../bin/solaris/fcast"
	;;

	FreeBSD)
	echo "Running FCAST Solaris Send/Recv Test"
	fcast_path="../../bin/freebsd/fcast"
	;;
	# other OS???? todo
esac

#
# multicast tests first
#

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv=''
#verbosity_send='-v5'		# sender part
verbosity_send=''

echo ""
echo "** Multicast tests..."
echo ""
echo "${fcast_path} ${verbosity_recv} -recv -a225.1.2.3/9991 -never -ospeed -phigh"
echo "${fcast_path} ${verbosity_send} -send -a225.1.2.3/9991 -ospeed -phigh -R ./"
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -a225.1.2.3/9991 -never -ospeed -phigh &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -a225.1.2.3/9991 -ospeed -phigh -R ./ &
fcast_send_pid=$!

wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??

cd ../..
diff -r ./fcast_test/send ./fcast_test/recv
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
# and now unicast tests...
#

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv=''
#verbosity_send='-v5'		# sender part
verbosity_send=''

echo ""
echo "** And unicast tests now..."
echo ""
echo "${fcast_path} ${verbosity_recv} -recv -a127.0.0.1/9998 -never"
echo "${fcast_path} ${verbosity_send} -send -a127.0.0.1/9998 -R ./"
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -a127.0.0.1/9998 -never &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -a127.0.0.1/9998 -R ./ &
fcast_send_pid=$!

wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??

cd ../..
diff -r ./fcast_test/send ./fcast_test/recv
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
