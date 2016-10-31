# TimerEx
Steve's Timer for embedded linux
TimerEx.c
TimerEx.h
Makefile

TO BUILD:
git clone https://github.com/steve12de/TimerEx.git
cd TimerEx
make

Creates Cross-compiled library for'arm-none-linux-gnueabi-gcc' at:
   : TimerEx/lib/timer/libas-timerEx.a

To Use Timer.
    #define TIMER1_EXPIRE       2000 // in ms
    static int rc;
   
    // create timer
    TimerEx timer1;
    timer1 = (timer_t*) INF_MALLOC(sizeof(timer_t));
    if ((rc = (TimerEx_makeTimer((char*)"timer1", timer1, TIMER1_EXPIRE, 0))) == -1)
    {
        fprintf(stderr,"----Timer1 Create Error\n" );
    }
    else
    {
        TimerEx_put_timer(timex, self->timer1, TIMER_ONE_SHOT);
    }
    
    // check timer expired
    if (TimerEx_retTimerSet(timex, timer1, &timerRef))
    {
        fprintf(stderr,"----Timer1:[%d] expire\n",timerRef);
        TimerEx_stoptimer(timex, timer1);
    }
    
    // re-start Timer
    TimerEx_starttimer(self->timex, self->timer1, TIMER1_EXPIRE, 0);
