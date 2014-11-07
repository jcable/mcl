#!/bin/sh
#
# $Id: test_fcast_mcl_modes.sh,v 1.4 2005/03/23 14:05:00 roca Exp $
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


#
# fcast validation script #3 : Fcast options and MCL Modes Tests
#
#
host_name=`uname -s`


case ${host_name} in

	Linux)
	echo "Running FCAST Linux Send/Recv Test #3"
	fcast_path="../../bin/linux/fcast"
	;;

	SunOS)
	echo "Running FCAST Solaris Send/Recv Test #3"
	fcast_path="../../bin/solaris/fcast"
	;;

	FreeBSD)
	echo "Running FCAST Solaris Send/Recv Test #3"
	fcast_path="../../bin/freebsd/fcast"
	;;
	# other OS???? todo
esac

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv=''
#verbosity_send='-v1'		# sender part
verbosity_send=''

# file sent
file='./Other_Files/i-am-a-BIG.file'


echo ""
echo "** Multicast tests only..."
echo ""

echo "--- Speed Optmimization Test..."
recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -force -phigh -ospeed -a225.1.2.3/9991 &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -phigh -ospeed -a225.1.2.3/9991 ${file} &
fcast_send_pid=$!


wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??


cd ../..
diff ./fcast_test/send/${file} ./fcast_test/recv/${file}
diff_val=$?

if [ ${recv_val} -ne 0 ]
then
	echo "FCAST Recv Failed"
	exit 1

elif [ ${send_val} -ne 0 ]
then
	echo "FCAST Send Failed"
	exit 1

elif [ ${diff_val} -ne 0 ]
then
	echo "Test failed: received files do not match sent files!"
	exit 1

else
	echo "-- Test OK!"
fi


echo "--- Space Optmimization Test..."
recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -force -phigh -ospace -a225.2.3.4/9992 &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -phigh -ospace -a225.2.3.4/9992 ${file} &
fcast_send_pid=$!


wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?

cd ../..
diff ./fcast_test/send/${file} ./fcast_test/recv/${file}
diff_val=$?

if [ ${recv_val} -ne 0 ]
then
        echo "FCAST Recv Failed"
        exit 1

elif [ ${send_val} -ne 0 ]
then
        echo "FCAST Send Failed"
        exit 1

elif [ ${diff_val} -ne 0 ]
then
        echo "Test failed: received files do not match sent files!"
        exit 1

else
        echo "-- Test OK!" 
fi

echo "--- CPU Optmimization Test..."
recv_val=1
send_val=1
rm -Rf ./fcast_test/recv/*
cd fcast_test/recv
../../${fcast_path} ${verbosity_recv} -recv -force -phigh -ocpu -a225.3.4.5/9993 &
fcast_recv_pid=$!

cd ../send
../../${fcast_path} ${verbosity_send} -send -phigh -ocpu -a225.3.4.5/9993 ${file} &
fcast_send_pid=$!


wait ${fcast_recv_pid}
recv_val=$?

wait ${fcast_send_pid}
send_val=$?
# TODO : what if Sender has finished before receiver ??


cd ../..
diff ./fcast_test/send/${file} ./fcast_test/recv/${file}
diff_val=$?


if [ ${recv_val} -ne 0 ]
then
        echo "FCAST Recv Failed"
        exit 1

elif [ ${send_val} -ne 0 ]
then
        echo "FCAST Send Failed"
        exit 1

elif [ ${diff_val} -ne 0 ]
then
        echo "Test failed: received files do not match sent files!"
        exit 1

else
        echo "-- Test OK!"
	exit 0
fi

