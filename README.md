# TimerEx
Steve's Timer for embedded linux
 -TimerEx.c
 -TimerEx.h
 -Makefile

Library Functions for any number of Application Timers
Build as a Library using make.

TO BUILD:
git clone https://github.com/steve12de/TimerEx.git
cd TimerEx
Set up make for compiler (e.g. gcc or arm-none-linux-gnueabi-gcc etc)
make  (Makefile creates lib/libas-timerEx.a)
For Test only: gcc --static -c timerEx.c -std=gnu99 -lrt -o timerEx

TO USE:
Create a structure (TimerEx) containing an array of tm_timers timers[TIMER_MAX]
Create a timer (one shot or restart) and add to TimerEx
Check whether timer expired or check timeout period (sec, usec)

FOR DETAILS OF HOW TO USE IN APPLICATIONS
See TimerEx.c   
