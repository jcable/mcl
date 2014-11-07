#!/bin/sh
#
# $Id: test_flute_RLC.sh,v 1.2 2005/05/12 16:03:20 moi Exp $
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

if [ -z $1 ]
then
	echo "You must specify the flute binary file of MADs Flute implementation."
	echo "Syntax: ./test_.._...sh mad_flute"
	exit 1
fi

if [ ! -x $1 ]
then
	echo "You specified a non executable file."
	echo "You must specify the flute binary file of MADs Flute implementation."
	echo "Syntax: ./test_.._...sh mad_flute"
	exit 1
fi


host_name=`uname -s`
host_network_name=`uname -n`
host_ip=`hostname -i`

case ${host_name} in

	Linux)
	echo "Running FLUTE Linux Send/Recv Test"
	flute_path="../../../bin/linux/flute"
	;;

	SunOS)
	echo "Running FLUTE Solaris Send/Recv Test"
	flute_path="../../../bin/solaris/flute"
	;;
	
	FreeBSD)
	echo "Running FLUTE Solaris Send/Recv Test"
	flute_path="../../../bin/freebsd/flute"
	;;	

	# other OS???? todo
esac

#
# MCL to MAD using RSE
#

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv='-stat1'		# has to be at least -v1 or stat1
#verbosity_send='-v5'		# sender part
verbosity_send='-stat1'		# has to be at least -v1 or stat1

echo ""
echo "** MCL to MAD using RLC"
echo ""
echo "$1 -A -m:225.1.2.3 -p:9991 -v:0 -w:1 -c:5 -s:${host_ip} -B:./"
echo "${flute_path} ${verbosity_recv} -send -cc1 -l5 -fec1.5 -a225.1.2.3/9991  -demux1 ./"
echo ""

recv_val=1
send_val=1
rm -Rf ../files/recv/*
cd ../files/recv
$1 -A -m:225.1.2.3 -p:9991 -v:0 -w:1 -c:5 -s:${host_ip} -B:./&
flute_recv_pid=$!

cd ../send
../${flute_path} ${verbosity_recv} -send -cc1 -l5 -fec1.0 -a225.1.2.3/9991 -demux1 ./ &
flute_send_pid=$!

wait ${flute_recv_pid}
recv_val=$?

wait ${flute_send_pid}
send_val=$?
# TODO : what if sender has finished before receiver ??

cd ../../iop
#diff -r ./files/send ./files/recv
diff ../files/send/ ../files/recv/

diff_val=$?

if [ ${send_val} -ne 0 ]
then
	echo "FLUTE Send Failed"
	exit 1

elif [ ${recv_val} -ne 253 ]
then
	echo "FLUTE Recv Failed"
	exit 1

elif [ ${diff_val} -ne 0 ]
then
	echo "Test failed: received files do not match sent files!"
	exit 1
fi




#
# MAD to MCL using RSE
#

# for debug...
#verbosity_recv='-v5'		# receiver part
verbosity_recv='-stat1'		# has to be at least -v1 or stat1
#verbosity_send='-v5'		# sender part
verbosity_send='-stat1'		# has to be at least -v1 or stat1

echo ""
echo "** MAD to MCL using RSE "
echo ""
echo "${flute_path} ${verbosity_recv} -recv -a225.1.2.3/9991 -demux1"
echo "$1 -S -m:225.1.2.3 -p:9991 -v:0 -x:2 -F:../"
echo ""

recv_val=1
send_val=1
rm -Rf ../files/recv/*
cd ../files/recv
../${flute_path} ${verbosity_recv} -recv -cc1 -l5 -a225.1.2.3/9991 -demux1 &
flute_recv_pid=$!

cd ..
$1 -S -m:225.1.2.3 -p:9991 -w:1 -c:5  -v:0 -F:./send &
flute_send_pid=$!

wait ${flute_recv_pid}
recv_val=$?

wait ${flute_send_pid}
send_val=$?
# TODO : what if sender has finished before receiver ??

cd ../iop
#diff -r ./files/send ./files/recv
diff ../files/send/ ../files/recv/${host_network_name}/send

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

