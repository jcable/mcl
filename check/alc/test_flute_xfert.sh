#!/bin/sh
#
# $Id: test_flute_xfert.sh,v 1.10 2005/02/23 17:32:35 moi Exp $
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
#echo "WARNING: test skipped !!!!!!"
#exit 0

#
# flute validation script #1 : Send/Recv Test
#
#
host_name=`uname -s`


case ${host_name} in

	Linux)
	echo "Running FLUTE Linux Send/Recv Test"
	flute_path="../../bin/linux/flute"
	;;

	SunOS)
	echo "Running FLUTE Solaris Send/Recv Test"
	flute_path="../../bin/solaris/flute"
	;;
	
	FreeBSD)
	echo "Running FLUTE Solaris Send/Recv Test"
	flute_path="../../bin/freebsd/flute"
	;;	

	# other OS???? todo
esac

#
# multicast tests first
#

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv='-stat1'		# has to be at least -v1 or stat1
#verbosity_send='-v5'		# sender part
verbosity_send='-stat1'		# has to be at least -v1 or stat1

echo ""
echo "** Multicast tests..."
echo ""
echo "${flute_path} ${verbosity_recv} -recv -a225.1.2.3/9991 -phigh"
echo "${flute_path} ${verbosity_send} -send -a225.1.2.3/9991 -phigh ./Other_Files/i-am-a-BIG.file"
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${flute_path} ${verbosity_recv} -recv -a225.1.2.3/9991 -phigh &
flute_recv_pid=$!

cd ../send
../../${flute_path} ${verbosity_send} -send -a225.1.2.3/9991 -phigh ./Other_Files/i-am-a-BIG.file &
flute_send_pid=$!

wait ${flute_recv_pid}
recv_val=$?

wait ${flute_send_pid}
send_val=$?
# TODO : what if sender has finished before receiver ??

cd ../..
#diff -r ./fcast_test/send ./fcast_test/recv
diff ./fcast_test/send/Other_Files/i-am-a-BIG.file ./fcast_test/recv/Other_Files/i-am-a-BIG.file

diff_val=$?


if [ ${send_val} -ne 0 ]
then
	echo "FLUTE Send Failed"
	exit 1

elif [ ${recv_val} -ne 0 ]
then
	echo "FLUTE Recv Failed"
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
verbosity_recv='-stat1'		# has to be at least -v1 or stat1
#verbosity_send='-v5'		# sender part
verbosity_send='-stat1'		# has to be at least -v1 or stat1

echo ""
echo "** And unicast tests now..."
echo ""
echo "${flute_path} ${verbosity_recv} -recv -a127.0.0.1/9998 -phigh"
echo "${flute_path} ${verbosity_send} -send -a127.0.0.1/9998 -phigh ./Other_Files/i-am-a-BIG.file"
echo ""

recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${flute_path} ${verbosity_recv} -recv -a127.0.0.1/9998 -phigh &
flute_recv_pid=$!

cd ../send
../../${flute_path} ${verbosity_send} -send -a127.0.0.1/9998 -phigh ./Other_Files/i-am-a-BIG.file &
flute_send_pid=$!

wait ${flute_recv_pid}
recv_val=$?

wait ${flute_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??

cd ../..
#diff -r ./fcast_test/send ./fcast_test/recv
diff ./fcast_test/send/Other_Files/i-am-a-BIG.file ./fcast_test/recv/Other_Files/i-am-a-BIG.file

diff_val=$?


# do not test send_val return value here as in unicast,
# the sender is always stopped as soon as the receiver leaves
# with a sendmsg error. This is normal so ignore it and only
# rely on diff tests
#
if [ ${recv_val} -ne 0 ]
then
	echo "FLUTE Recv Failed"
	exit 1

elif [ ${diff_val} -ne 0 ]
then
	echo "Test failed: received files do not match sent files!"
	exit 1

else
	exit 0
fi
