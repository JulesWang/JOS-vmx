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

	readline_hack=`grep 'readline_hack$' obj/kernel.sym | sed -e's/ .*$//g' | sed -e's/^f//'`

	# Run Bochs, setting a breakpoint at readline(),
	# and feeding in appropriate commands to run, then quit.
	(
		# The sleeps are necessary in some Bochs to
		# make it parse each line separately.  Sleeping
		# here sure beats waiting for the timeout.
		echo vbreak 0x8:0x$brkaddr
		psleep .1
		echo c
		if test "$readline_hackval" != 0; then
			psleep .1
			echo setpmem 0x$readline_hack 1 $readline_hackval
			psleep .1
			echo c
		fi	
		# EOF will do just fine to quit.
	) | (
		ulimit -t $timeout
		bochs -q 'display_library: nogui' \
			'parport1: enabled=1, file="bochs.out"'
	) >$out 2>$err
}



readline_hackval=1
gmake
runbochs

score=0

echo_n "Page directory: "
 if grep "boot_mem_check() succeeded!" bochs.out >/dev/null
 then
	score=`expr 20 + $score`
	echo OK
 else
	echo WRONG
 fi

echo_n "Page management: "
 if grep "page_check() succeeded!" bochs.out >/dev/null
 then
	score=`expr 30 + $score`
	echo OK
 else
	echo WRONG
 fi

echo_n "Kernel breakpoint interrupt: "
 if grep "^TRAP frame at 0x" bochs.out >/dev/null \
     && grep "  trap 0x00000003 Breakpoint" bochs.out >/dev/null
 then
	score=`expr 10 + $score`
	echo OK
 else
	echo WRONG
 fi

echo_n "Returning from breakpoint interrupt: "
 if grep "Breakpoint succeeded!" bochs.out >/dev/null
 then
	score=`expr 10 + $score`
	echo OK
 else
	echo WRONG
 fi

echo "Score: $score/70"


