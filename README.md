JOS
-----
JOS is a teaching OS using by MIT/Stanford/UCLA OS course. There are several hands-on exercises in it. 

The code in this git repo comes from UCLA OS course website:
http://www.cs.ucla.edu/~kohler/class/06f-aos/index.html

JOS-vmx
-------
JOS-vmx(JOS Virtual Machine eXtensions) is another hands-on exercise, which aims at turning JOS into a hypervisor by utilizing hardware assisted virtualization(ex. Intel-vt).

Prerequisite
-------
* Have read the Chapter 19 INTRODUCTION TO VIRTUAL-MACHINE EXTENSIONS of Intel Software Developer's Manual Volume 3B System Programming Guide, Part 2(3B in brief).
* Have finished JOS Lab2.
* Have known what is the **Matrix**. 
* Have EPT and Unrestricted guest features present in the CPU. This is not necessary if you use bochs emulator instead.

Brief note
-------
* Step1: > make
* Step2: > make install-grub
* Step3: Add a JOS entry in your system grub list(eg. /boot/grub/menu.lst)

Something like this:
<pre>
title JOS-vmx
        root (hd0,0)
        kernel /jos
</pre>
* Step4: Reboot your computer
* Step5: Enter JOS in grub

When Ex1 - Ex5 are completed

* Step6: Type matrix in JOS command line:

>  K>matrix

> You will create a VM which contains JOS itself, so don't be panic when the system seems to be rebooting. It is not rebooting, it is just booting the VM.

* Step7: Enter JOS in grub in the matrix
* Step8: Type cpuid in JOS command line

> K>cpuid

> You will get 'deadbeef'. If you get that, it means that you are in the Matrix.

alternative(not recommended)

* Step1: Download bochs-2.4.5
* Step2: ./configure --enable-debugger  --enable-vmx=2 --enable-x86-64 --prefix=/usr
* Step3: make & make install
* The following process is just the same as what we did in JOS lab.

Sayings
-------

"When you have learned to snatch the error code from the trap frame, it will be time for you to leave." --- 《The Tao Of Programming》


"When you have learned to create the MATRIX, it will be time for you to leave." --- Jules Wang

JOS CODING STANDARDS
--------------------

It's easier on everyone if all authors working on a shared
code base are consistent in the way they write their programs.
We have the following conventions in our code:

* No space after the name of a function in a call
  For example, printf("hello") **not** printf ("hello").

* One space after keywords "if", "for", "while", "switch".
  For example, if (x) **not** if(x).

* Space before braces.
  For example, if (x) { **not** if (x){.

* Function names are all lower-case separated by underscores.

* Beginning-of-line indentation via tabs, **not** spaces.

* Preprocessor macros are always UPPERCASE.
  There are a few grandfathered exceptions: assert, panic,
  static_assert, offsetof.

* Pointer types have spaces: (uint16_t \*) **not** (uint16_t\*).

* Multi-word names are lower_case_with_underscores.

* Comments in imported code are usually C /* ... */ comments.
  Comments in new code are C++ style //.

* In a function definition, the function name starts a new line.
  Then you can grep -n '^foo' */*.c to find the definition of foo.

* Functions that take no arguments are declared f(void) **not** f().
