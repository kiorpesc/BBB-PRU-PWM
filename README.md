BBB-PRU-PWM
===========

PWM implemented using the BeagleBone Black's PRU

This can currently output a single channel of PWM at various frequencies (user configurable).

To use, you first need to get the PRU tools.  Instructions can be found at:
http://www.element14.com/community/community/knode/single-board_computers/next-gen_beaglebone/blog/2013/05/22/bbb--working-with-the-pru-icssprussv2
Step 2 details how to install and compile the assembler, and add it to your libraries.

You will also need to enable the PRU on the BBB first.

       modprobe uio_pruss
       echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots

Then, simply type:

       make
       ./pru_pwm

The PWM output will start at 50Hz.  You can enter values in microseconds, and the pulse width will change accordingly.
If you enter 0, the program will exit cleanly, halting and shutting down the PRU and clearing mapped memory.

You can configure another frequency (up to ~490Hz) by changing PERIOD_CYCLES in the C code.
Right now you will need to do the calculation to convert it into a count of PRU clock cycles:
       (1/(desired Hz)) / .000000005 = PERIOD_CYCLES
