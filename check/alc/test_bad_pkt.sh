#!/bin/sh

# skipped !
echo "WARNING: test skipped !!!!!!"
exit 0

PATH=~/work/mcl/bin/linux/

mclrecv2 -v1 -a230.1.2.3/2323 &
recv_pid=$!


for i in 0 1 2 3 4 5 6 7 8 9 10 11 12
do 
	./test_bad_pkt 230.1.2.3 2323 $i
	test_pid=$!

done

kill -9 -s SIGKILL ${recv_pid}
wait ${recv_pid}
val=$?
if [ ${val} -ne 0 ] ; then
	echo ""
	echo "---------- ERROR: Test failed!----------"
	echo "mclrecv2 returned ${val}\n"
	exit 1
fi
