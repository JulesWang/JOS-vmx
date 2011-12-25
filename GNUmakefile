#
# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#	Recursive Make Considered Harmful
#	http://aegis.sourceforge.net/auug97.pdf
#
OBJDIR := obj

ifdef LAB
SETTINGLAB := true
else
-include conf/lab.mk
endif

-include conf/env.mk

ifndef SOL
SOL := 0
endif
ifndef LABADJUST
LABADJUST := 0
endif

ifndef LABSETUP
LABSETUP := ./
endif


TOP = .

# Cross-compiler jos toolchain
#
# This Makefile will automatically use the cross-compiler toolchain
# installed as 'i386-jos-elf-*', if one exists.  If the host tools ('gcc',
# 'objdump', and so forth) compile for a 32-bit x86 ELF target, that will
# be detected as well.  If you have the right compiler toolchain installed
# using a different name, set GCCPREFIX explicitly by doing
#
#	make 'GCCPREFIX=i386-jos-elf-' gccsetup

# try to infer the correct GCCPREFIX
ifndef GCCPREFIX
GCCPREFIX := $(shell if i386-jos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-jos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-jos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-jos-elf-', set your GCCPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake GCCPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

CC	:= $(GCCPREFIX)gcc -pipe
GCC_LIB := $(shell $(CC) -print-libgcc-file-name)
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
OBJCOPY	:= $(GCCPREFIX)objcopy
OBJDUMP	:= $(GCCPREFIX)objdump
NM	:= $(GCCPREFIX)nm

# Native commands
NCC	:= gcc $(CC_VER) -pipe
TAR	:= gtar
PERL	:= perl

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS	:= $(CFLAGS) $(DEFS) $(LABDEFS) -O -fno-builtin -I$(TOP) -MD -Wall -Wno-format -Wno-unused -Werror -gstabs

# Linker flags for JOS user programs
ULDFLAGS := -T user/user.ld

# Lists that the */Makefrag makefile fragments will add to
OBJDIRS :=

# Make sure that 'all' is the first target
all:

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o $(OBJDIR)/boot/%.o $(OBJDIR)/kern/%.o \
	$(OBJDIR)/lib/%.o $(OBJDIR)/fs/%.o $(OBJDIR)/user/%.o

KERN_CFLAGS := $(CFLAGS) -DJOS_KERNEL -gstabs
USER_CFLAGS := $(CFLAGS) -DJOS_USER -gstabs


CXX := $(GCCPREFIX)c++ -pipe
KERN_CXXFLAGS := $(KERN_CFLAGS) -fno-exceptions -fno-rtti
USER_CXXFLAGS := $(USER_CFLAGS) -fno-exceptions -fno-rtti



# Include Makefrags for subdirectories
include boot/Makefrag
include kern/Makefrag
include lib/Makefrag
include user/Makefrag


IMAGES = $(OBJDIR)/kernel.img

bochs: $(IMAGES)
	bochs 'display_library: nogui'

# For deleting the build
clean:
	rm -rf $(OBJDIR) kern/programs.c

realclean: clean
	rm -rf lab$(LAB).tar.gz bochs.out bochs.log

distclean: realclean
	rm -rf conf/gcc.mk

grade: $(LABSETUP)grade.sh
	$(V)$(MAKE) clean >/dev/null 2>/dev/null
	$(MAKE) all
	sh $(LABSETUP)grade.sh

tarball: realclean
	tar cf - `ls -a | grep -v '^\.*$$' | grep -v '^CVS$$' | grep -v '^lab[0-9].*\.tar\.gz'` | gzip > lab$(LAB)-$(USER).tar.gz

# For test runs
xrun:
	$(V)$(MAKE) $(IMAGES)
	bochs -q

run-%:
	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
	bochs -q 'display_library: nogui'

xrun-%:
	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
	bochs -q

# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	@$(PERL) mergedep.pl $@ $^

-include $(OBJDIR)/.deps

# Create a patch from ../lab$(LAB).tar.gz.
diff-extract-tarball:
	@test -r ../lab$(LAB).tar.gz || (echo "***" 1>&2; \
	echo "*** Can't find '../lab$(LAB).tar.gz'.  Download it" 1>&2; \
	echo "*** into my parent directory and try again." 1>&2; \
	echo "***" 1>&2; false)
	(gzcat ../lab$(LAB).tar.gz 2>/dev/null || zcat ../lab$(LAB).tar.gz) | tar xf -

diff-check-date: diff-extract-tarball
	@pkgdate=`grep PACKAGEDATE lab$(LAB)/conf/lab.mk | sed 's/.*=//'`; \
	test "$(PACKAGEDATE)" = "$$pkgdate" || (echo "***" 1>&2; \
	echo "*** The ../lab$(LAB).tar.gz tarball was created on $$pkgdate," 1>&2; \
	echo "*** but your work directory was expanded from a tarball created" 1>&2; \
	echo "*** on $(PACKAGEDATE)!  I can't tell the difference" 1>&2; \
	echo "*** between your changes and the changes between the tarballs," 1>&2; \
	echo "*** so I won't create an automatic patch." 1>&2; \
	echo "***" 1>&2; false)

changes.patch: diff-extract-tarball always
	@rm -f changes.patch
	@for f in `cd lab$(LAB) && find . -type f -print`; do \
	if diff -u lab$(LAB)/"$$f" "$$f" >changes.patchpart || [ "$$f" = ./boot/lab.mk ]; then :; else \
	echo "*** $$f differs; appending to changes.patch" 1>&2; \
	echo diff -u lab$(LAB)/"$$f" "$$f" >>changes.patch; \
	cat changes.patchpart >>changes.patch; \
	fi; done
	@for f in `find . -name lab$(LAB) -prune -o -name obj -prune -o -name "changes.patch*" -prune -o -name '*.rej' -prune -o -name '*.orig' -prune -o -type f -print`; do \
	if [ '(' '!' -f lab$(LAB)/"$$f" ')' -a '(' "$$f" != ./kern/programs.c ')' -a '(' "$$f" != ./conf/gcc.mk ')' ]; then \
	echo "*** $$f is new; appending to changes.patch" 1>&2; \
	echo New file: "$$f" >>changes.patch; \
	echo diff -u lab$(LAB)/"$$f" "$$f" >>changes.patch; \
	echo '--- lab$(LAB)/'"$$f"'	Thu Jan 01 00:00:00 1970' >>changes.patch; \
	diff -u /dev/null "$$f" | tail +2 >>changes.patch; \
	fi; done
	@test -n changes.patch || echo "*** No differences found" 1>&2
	@rm -rf lab$(LAB) changes.patchpart

diff: diff-check-date changes.patch

patch:
	@test -r changes.patch || (echo "***" 1>&2; \
	echo "*** No 'changes.patch' file found!  Did you remember to" 1>&2; \
	echo "*** run 'make patch'?" 1>&2; \
	echo "***" 1>&2; false)
	@x=`grep "^New file:" changes.patch | head -n 1 | sed 's/New file: //'`; \
	if test -n "$$x" -a -f "$$x"; then \
	echo "*** Note: File '$$x' found in current directory;" 1>&2; \
	echo "*** not applying new files portion of changes.patch." 1>&2; \
	echo "awk '/^New file:/ { exit } { print }' <changes.patch | patch -p0"; \
	awk '/^New file:/ { exit } { print }' <changes.patch | patch -p0; \
	else echo 'patch -p0 <changes.patch'; \
	patch -p0 <changes.patch; \
	fi

always:
	@:

.PHONY: all always diff patch \
	diff-extract-tarball diff-check-date \
	tarball clean realclean clean-labsetup distclean grade labsetup
