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


# Usage: runtest <tagname> <defs> <strings...>
runtest () {
	perl -e "print '$1: '"
	rm -f obj/kern/init.o obj/kernel obj/kernel.img 
	[ "$preservefs" = y ] || rm -f obj/fs.img
	if $verbose
	then
		echo "gmake $2... "
	fi
	gmake $2 >$out
	if [ $? -ne 0 ]
	then
		echo gmake $2 failed 
		exit 1
	fi
	runbochs
	if [ ! -s bochs.out ]
	then
		echo 'no bochs.out'
	else
		shift
		shift
		continuetest "$@"
	fi
}

quicktest () {
	perl -e "print '$1: '"
	shift
	continuetest "$@"
}

continuetest () {
	okay=yes

	not=false
	for i
	do
		if [ "x$i" = "x!" ]
		then
			not=true
		elif $not
		then
			if egrep "^$i\$" bochs.out >/dev/null
			then
				echo "got unexpected line '$i'"
				if $verbose
				then
					exit 1
				fi
				okay=no
			fi
			not=false
		else
			egrep "^$i\$" bochs.out >/dev/null
			if [ $? -ne 0 ]
			then
				echo "missing '$i'"
				if $verbose
				then
					exit 1
				fi
				okay=no
			fi
			not=false
		fi
	done
	if [ "$okay" = "yes" ]
	then
		score=`expr $pts + $score`
		echo OK
	else
		echo WRONG
	fi
}

# Usage: runtest1 [-tag <tagname>] <progname> [-Ddef...] STRINGS...
runtest1 () {
	if [ $1 = -tag ]
	then
		shift
		tag=$1
		prog=$2
		shift
		shift
	else
		tag=$1
		prog=$1
		shift
	fi
	runtest1_defs=
	while expr "x$1" : 'x-D.*' >/dev/null; do
		runtest1_defs="DEFS+='$1' $runtest1_defs"
		shift
	done
	runtest "$tag" "DEFS='-DTEST=_binary_obj_user_${prog}_start' DEFS+='-DTESTSIZE=_binary_obj_user_${prog}_size' $runtest1_defs" "$@"
}



score=0

runtest1 hello -DJOS_MULTIENV=0 \
	'.00000000. new env 00001000' \
	'hello, world' \
	'i am environment 00001000' \
	'.00001000. exiting gracefully' \
	'.00001000. free env 00001000' \
	'Destroyed all environments - nothing more to do!'

# the [00001000] tags should have [] in them, but that's 
# a regular expression reserved character, and i'll be damned if
# I can figure out how many \ i need to add to get through 
# however many times the shell interprets this string.  sigh.

runtest1 buggyhello -DJOS_MULTIENV=0 \
	'.00001000. user_mem_check va 00000...' \
	'.00001000. free env 00001000'

runtest1 evilhello -DJOS_MULTIENV=0 \
	'.00001000. user_mem_check va f0100...' \
	'.00001000. free env 00001000'

runtest1 divzero -DJOS_MULTIENV=0 \
	! '1/0 is ........!' \
	'Incoming TRAP frame at 0xefbfff..' \
	'  trap 0x00000000 Divide error' \
	'  eip  0x008.....' \
	'  ss   0x----0023' \
	'.00001000. free env 00001000'

runtest1 breakpoint -DJOS_MULTIENV=0 \
	'Welcome to the JOS kernel monitor!' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x00000003 Breakpoint' \
	'  eip  0x008.....' \
	'  ss   0x----0023' \
	! '.00001000. free env 00001000'

runtest1 softint -DJOS_MULTIENV=0 \
	'Welcome to the JOS kernel monitor!' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000d General Protection' \
	'  eip  0x008.....' \
	'  ss   0x----0023' \
	'.00001000. free env 00001000'

runtest1 badsegment -DJOS_MULTIENV=0 \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000d General Protection' \
	'  err  0x0000001c' \
	'  eip  0x008.....' \
	'  ss   0x----0023' \
	'.00001000. free env 00001000'

runtest1 faultread -DJOS_MULTIENV=0 \
	! 'I read ........ from location 0!' \
	'.00001000. user fault va 00000000 ip 008.....' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000004' \
	'.00001000. free env 00001000'

runtest1 faultreadkernel -DJOS_MULTIENV=0 \
	! 'I read ........ from location 0xf0100000!' \
	'.00001000. user fault va f0100000 ip 008.....' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000005' \
	'.00001000. free env 00001000' \

runtest1 faultwrite -DJOS_MULTIENV=0 \
	'.00001000. user fault va 00000000 ip 008.....' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000006' \
	'.00001000. free env 00001000'

runtest1 faultwritekernel -DJOS_MULTIENV=0 \
	'.00001000. user fault va f0100000 ip 008.....' \
	'Incoming TRAP frame at 0xefbfffbc' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000007' \
	'.00001000. free env 00001000'

runtest1 testbss -DJOS_MULTIENV=0 \
	'Making sure bss works right...' \
	'Yes, good.  Now doing a wild write off the end...' \
	'.00001000. user fault va 00c..... ip 008.....' \
	'.00001000. free env 00001000'

pts=30
runtest1 dumbfork \
	'.00000000. new env 00001000' \
	'.00000000. new env 00001001' \
	'0: I am the parent!' \
	'9: I am the parent!' \
	'0: I am the child!' \
	'9: I am the child!' \
	'19: I am the child!' \
	'.00001001. exiting gracefully' \
	'.00001001. free env 00001001' \
	'.00001002. exiting gracefully' \
	'.00001002. free env 00001002'

pts=10
readline_hackval=1
runtest1 -tag 'breakpoint [backtrace]' breakpoint -DJOS_MULTIENV=0 \
	'^Stack backtrace:' \
	' *user/breakpoint.c:.*' \
	' *lib/libmain.c:.*' \
	' *lib/entry.S:.*'

echo Score: $score/100



