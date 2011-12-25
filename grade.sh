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

if gmake --version >/dev/null 2>&1; then make=gmake; else make=make; fi

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
		psleep .2
		echo c
		if test "$readline_hackval" != 0; then
			psleep .2
			echo setpmem 0x$readline_hack 1 $readline_hackval
			psleep .2
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
		echo "$make $2... "
	fi
	$make $2 >$out
	if [ $? -ne 0 ]
	then
		echo $make $2 failed 
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
timeout=10

runtest1 faultread \
	! 'I read ........ from location 0!' \
	'.00001001. user fault va 00000000 ip 008.....' \
	'TRAP frame at.*' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000004' \
	'.00001001. free env 00001001'

runtest1 faultwrite \
	'.00001001. user fault va 00000000 ip 008.....' \
	'TRAP frame at.*' \
	'  trap 0x0000000e Page Fault' \
	'  err  0x00000006' \
	'.00001001. free env 00001001'

runtest1 faultdie \
	'i faulted at va deadbeef, err 6' \
	'.00001001. exiting gracefully' \
	'.00001001. free env 00001001' 

runtest1 faultalloc \
	'fault deadbeef' \
	'this string was faulted in at deadbeef' \
	'fault cafebffe' \
	'fault cafec000' \
	'this string was faulted in at cafebffe' \
	'.00001001. exiting gracefully' \
	'.00001001. free env 00001001'

runtest1 faultallocbad \
	'.00001001. user_mem_check va deadbeef' \
	'.00001001. free env 00001001' 

runtest1 faultnostack \
	'.00001001. user_mem_check va eebfff..' \
	'.00001001. free env 00001001'

runtest1 faultbadhandler \
	'.00001001. user_mem_check va eebfef..' \
	'.00001001. free env 00001001'

runtest1 faultevilhandler \
	'.00001001. user_mem_check va eebfef..' \
	'.00001001. free env 00001001'

runtest1 forktree \
	'....: I am .0.' \
	'....: I am .1.' \
	'....: I am .000.' \
	'....: I am .100.' \
	'....: I am .110.' \
	'....: I am .111.' \
	'....: I am .011.' \
	'....: I am .001.' \
	'.00002001. exiting gracefully' \
	'.0000100.. exiting gracefully' \
	'.0000100.. exiting gracefully' \
	'.0000300.. exiting gracefully' \
	'.0000200.. free env 0000200.'

echo PART 1 SCORE: $score/45

pts=10
runtest1 spin \
	'.00000000. new env 00001000' \
	'.00000000. new env 00001001' \
	'I am the parent.  Forking the child...' \
	'.00001001. new env 00001002' \
	'I am the parent.  Running the child...' \
	'I am the child.  Spinning...' \
	'I am the parent.  Killing the child...' \
	'.00001001. destroying 00001002' \
	'.00001001. free env 00001002' \
	'.00001001. exiting gracefully' \
	'.00001001. free env 00001001'

runtest1 pingpong \
	'.00000000. new env 00001000' \
	'.00000000. new env 00001001' \
	'.00001001. new env 00001002' \
	'send 0 from 1001 to 1002' \
	'1002 got 0 from 1001' \
	'1001 got 1 from 1002' \
	'1002 got 8 from 1001' \
	'1001 got 9 from 1002' \
	'1002 got 10 from 1001' \
	'.00001001. exiting gracefully' \
	'.00001001. free env 00001001' \
	'.00001002. exiting gracefully' \
	'.00001002. free env 00001002' \

runtest1 primes \
	'.00000000. new env 00001000' \
	'.00000000. new env 00001001' \
	'.00001001. new env 00001002' \
	'2 .00001002. new env 00001003' \
	'3 .00001003. new env 00001004' \
	'5 .00001004. new env 00001005' \
	'7 .00001005. new env 00001006' \
	'11 .00001006. new env 00001007' 

echo PART 1+2 SCORE: $score/75

pts=15
runtest1 spawnhello \
	'.00000000. new env 00001001' \
	'i am parent environment 00001001' \
	'.00001001. new env 00001002' \
	'hello, world' \
	'i am environment 00001002' \
	'.00001002. exiting gracefully'

runtest1 spawninit \
	'.00000000. new env 00001001' \
	'i am parent environment 00001001' \
	'.00001001. new env 00001002' \
	'init: running' \
	'init: data seems okay' \
	'init: bss seems okay' \
	'init: args: .init. .one. .two.' \
	'init: exiting' \
	'.00001002. exiting gracefully'

echo PART 1+2+3 SCORE: $score/105



