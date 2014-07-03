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

> Congratulations!


alternative(not recommended, using this way only if you have no suitable machine)

Install Bochs(A PC emulator)
* Download bochs-2.4.5
* > cd bochs-2.4.5
* > ./configure --enable-debugger  --enable-vmx=2 --enable-x86-64 --prefix=/usr
* > make & make install
* The following process is just the same as what we did in JOS lab.

Further Readings
-------

[CSE 591, Spring 2014: Special Topics in Computer Science: Virtualization](http://www.cs.stonybrook.edu/~porter/courses/cse591/s14/index.html)



Sayings
-------

"When you have learned to snatch the error code from the trap frame, it will be time for you to leave." --- 《The Tao Of Programming》


"When you have learned to create the MATRIX, it will be time for you to leave." --- Jules Wang
