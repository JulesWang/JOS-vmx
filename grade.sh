#! /bin/sh

verbose=false

if [ "x$1" = "x-v" ]
then
	verbose=true
	out=/dev/stdout
	err=/dev/stderr
else
	out=/dev/null
	err=/dev/null
fi

pts=5
timeout=30
preservefs=n
readline_hackval=0

echo_n () {
	# suns can't echo -n, and Mac OS X can't echo "x\c"
	echo "$@" | tr -d '
'
}

psleep () {
	# solaris "sleep" doesn't take fractions
	perl -e "select(undef, undef, undef, $1);"
}

runbochs () {
	# Find the address of the kernel readline function,
	# which the kernel monitor uses to read commands interactively.
	brkaddr=`grep 'readline$' obj/kernel.sym | sed -e's/ .*$//g'`
	#echo "brkaddr $brkaddr"

	# Run Bochs, setting a breakpoint at readline(),
	# and feeding in appropriate commands to run, then quit.
	(
		# The sleeps are necessary in some Bochs to
		# make it parse each line separately.  Sleeping
		# here sure beats waiting for the timeout.
		echo vbreak 0x8:0x$brkaddr
		psleep .1
		echo c
		# EOF will do just fine to quit.
	) | (
		ulimit -t $timeout
		bochs -q 'display_library: nogui' \
			'parport1: enabled=1, file="bochs.out"'
	) >$out 2>$err
}



gmake
runbochs

score=0

	echo_n "Printf: "
	if grep "6828 decimal is 15254 octal!" bochs.out >/dev/null
	then
		score=`expr 20 + $score`
		echo OK
	else
		echo WRONG
	fi

	echo_n "Backtrace: "
	cnt=`grep "ebp f01.* eip f01.* args" bochs.out | wc -l`
	if [ $cnt -eq 8 ]
	then
		score=`expr 15 + $score`
		echo_n "Count OK"
	else
		echo_n "Count WRONG"
	fi

	cnt=`grep "ebp f01.* eip f0100.* args" bochs.out | awk 'BEGIN { FS = ORS = " " }
{ print $7 }
END { printf("\n") }' | grep '^00000000 00000000 00000001 00000002 00000003 00000004 00000005' | wc -w`
	if [ $cnt -eq 8 ]; then
		score=`expr 15 + $score`
		echo , Args OK
	else
		echo , Args WRONG
	fi

	echo_n "Debugging symbols: "
	cnt=`grep "kern/init.c.*test_backtrace.*1 arg)" bochs.out | wc -l`
	if [ $cnt -eq 6 ]; then
		score=`expr 25 + $score`
		echo OK
	else
		echo WRONG
	fi

echo "Score: $score/75"

if [ $score -lt 75 ]; then
	exit 1
fi


