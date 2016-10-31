//****************************************************************************
// File:             timerEx.c
//****************************************************************************

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <stdbool.h>

#include "inc/timerEx.h"

/*************************** DATA DECLARATION ***********************/
static unsigned int timer_val;
// Required global Pointer for use in handler
TimerEx *timerExPtr;

/************************ FUNCTION PROTOTYPES ***********************/
void TimerEx_timerHandler(int, siginfo_t*, void*);

/*************************** FUNCTIONS ******************************/
/*
    TimerEx_construct
*/
void TimerEx_construct(TimerEx* self)
{
    int i;
    self->numTimer = 0;
    for (i=0; i< TIMER_MAX; i++)
    {
        self->timers[i].timer_id = 0;
        self->timers[i].timer_act = TIMER_ONE_SHOT;
        self->timers[i].timer_bit = 0;
        self->timers[i].tv.tv_sec = 0;
        self->timers[i].tv.tv_usec = 0;
        // set timers
        self->timers[i].timer_start_sec  = 0;
        self->timers[i].timer_start_usec = 0;
        self->timers[i].timer_end_sec    = 0;
        self->timers[i].timer_end_usec   = 0;
    }
    timer_val = 0;
    //set global pointer
    timerExPtr = self;
}

/*
    TimerEx_makeTimer
*/
int TimerEx_makeTimer(char *name, timer_t *timerID, unsigned int expireMS, unsigned int intervalMS )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = TimerEx_timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        //perror("sigaction");
        fprintf(stderr,"----TimerEx: Failed to setup signal handling for %s\n",name);
        return(-1);
    }
    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    if ((timer_create(CLOCK_REALTIME, &te, timerID)) == -1)
    {
        //perror("timer_create");
       fprintf(stderr,"----TimerEx: Error, Failed to create timer for %s errno:%d -(%s)\n",
            name, errno, strerror(errno));
        return(-1);
    }
    // first expire
    its.it_value.tv_sec = (time_t)(expireMS/1000);
    its.it_value.tv_nsec = (expireMS % 1000) * 1000000;
    //interval
    if (intervalMS == 0)
    {
        // Expire once only
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }
    else
    {
    its.it_interval.tv_sec = (time_t)(intervalMS/1000);
    its.it_interval.tv_nsec = (intervalMS % 1000) * 1000000;
    }
    fprintf(stderr,"----TimerEx MakeTimer for '%s'-sec=%d,ns=%lu\n",name,
           (int)its.it_value.tv_sec, its.it_value.tv_nsec);
    // start the timer
    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    if ((timer_settime(*timerID, 0, &its, NULL)) == -1)
    {
        //perror("timer_settime");
        return (-1);
    }
    else
    {
        fprintf(stderr,"----TimerEx MakeTimer: Start Timer ID:[%d] at sec:%d us:%lu\n", 
               (int)timerID, (int)tv.tv_sec,(long)tv.tv_usec);
    }
    return(0);
}

/*
    TimerEx_put_timer
*/
void TimerEx_put_timer(TimerEx* self, timer_t *timer_id, timer_action act)
{
    // restrict to TIMER_MAX timers
    if (self->numTimer < TIMER_MAX)
    {
        self->timers[self->numTimer].timer_id = *timer_id;
        self->timers[self->numTimer].timer_act = act;
        self->timers[self->numTimer].timer_bit = self->numTimer;
        //fprintf(stderr,"----TimerEx Put timer:%d Id @ %p\n", self->numTimer,(void*)self->timers[self->numTimer].timer_id);
        self->numTimer++;
    }
    else
    {
         fprintf(stderr,"----TimerEx: error - exceeded maximum timers\n");
    }
}

/*
    TimerEx_timerHandler
*/
void TimerEx_timerHandler( int sig, siginfo_t *si, void *uc )
{
    int i;
    timer_t *tidp;
    bool timerExpire = false;
    tidp = (timer_t*)si->si_value.sival_ptr;

    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    for (i = 0; i < TIMER_MAX; i++)
    {
        // use global pointer
         if ( *tidp == ((void*)(timerExPtr->timers[i].timer_id)) )
        {    
            timerExpire = true;
            timer_val |= (0x1 << i);  // set the timer that expired
            timerExPtr->timers[i].tv.tv_sec  = tv.tv_sec;
            timerExPtr->timers[i].tv.tv_usec = tv.tv_usec;
            //Don't call printf in handler, apart from debug puposes
            fprintf(stderr,"------TimerEx: Timer:[%d] expire at sec:%d us:%lu\n",
                    i,(int)tv.tv_sec,(long)tv.tv_usec);
            break;
        }
    }  // for
    if (!timerExpire)
    {
        fprintf(stderr,"----TimerEx: Unknown Timer Expire *tidp=%p\n",*tidp);
    }
}
/*
    TimerEx_retTimerSet
*/
bool TimerEx_retTimerSet(TimerEx* self, timer_t *timer_id, int *timerRef)
{
    // Match the timer ID and return the timer number (for shift test)
    int i;
    bool ret_val = false;
    for (i = 0; i < self->numTimer; i++)
    {
        // Find the timer in timers structure
        if ( (void*)*timer_id == (void*)self->timers[i].timer_id )
        {    
            *timerRef = self->timers[i].timer_bit;
            int shift = 0x1 << *timerRef;
            if (timer_val & shift)
            {
                //fprintf(stderr,"----TimerEx: timer:%d is set, timer_val:%d\n",i, timer_val);
                // clear bit
                timer_val = timer_val & ~shift;
                ret_val = true;
                break;
            }
        }
    }
    return (ret_val);
}

/*
    TimerEx_starttimer
*/
bool TimerEx_starttimer(TimerEx* self, timer_t *timer_id, unsigned int expireMS, unsigned int intervalMS )
{
    int timerRef;
    bool ret_val = true;
    struct itimerspec its;
    TimerEx_retTimerSet(self, timer_id, &timerRef);

    // first expire
    its.it_value.tv_sec = (time_t)(expireMS/1000);
    its.it_value.tv_nsec = (expireMS % 1000) * 1000000;
    //interval
    if (intervalMS == 0)
    {
        // Expire once only
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }
    else
    {
        its.it_interval.tv_sec = (time_t)(intervalMS/1000);
        its.it_interval.tv_nsec = (intervalMS % 1000) * 1000000;
    }
    // start the timer
    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    if ((timer_settime(*timer_id, 0, &its, NULL)) == -1)
    {
        //perror("timer_settime");
        fprintf(stderr,"----TimerEx: Timer:[%d] timer_settime failed:%d -(%s)\n",
        timerRef, errno, strerror(errno));
        ret_val = false;
    }
    else // return 0
    {
        fprintf(stderr,"----TimerEx: Start Timer:[%d] at sec:%d us:%lu\n", 
               (int)timerRef, (long)tv.tv_sec,(long)tv.tv_usec);
        self->timers[timerRef].timer_start_sec  = (long)tv.tv_sec;
        self->timers[timerRef].timer_start_usec = (long)tv.tv_usec;
        self->timers[timerRef].timer_end_sec    = 0;
        self->timers[timerRef].timer_end_usec   = 0;
    }
    return (ret_val);
}

/*
    TimerEx_stoptimer
*/
bool TimerEx_stoptimer(TimerEx* self, timer_t *timer_id)
{
    int timerRef;
    bool ret_val = true;
    struct itimerspec its;
    TimerEx_retTimerSet(self, timer_id, &timerRef);
    fprintf(stderr,"----TimerEx: Stop timer:%d\n", timerRef);
    if ((timer_gettime(*timer_id, &its)) != 0)
    {
       fprintf(stderr,"----TimerEx: Timer:[%d] timer_gettime failed:%d -(%s)\n",
                timerRef, errno, strerror(errno));
    }
    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    fprintf(stderr,"----TimerEx: Timer:[%d] stopped at sec:%d us:%lu\n", 
               (int)timerRef, (long)tv.tv_sec,(long)tv.tv_usec);
    self->timers[timerRef].timer_end_sec    = (long int)tv.tv_sec;
    self->timers[timerRef].timer_end_usec   = (long int)tv.tv_usec;
 
    // disarm the timer
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    if ((timer_settime(*timer_id, 0, &its, NULL)) != 0)
    {
       fprintf(stderr,"----TimerEx: Timer:[%d] timer_settime failed:%d -(%s)\n",
                timerRef, errno, strerror(errno));
       ret_val = false;
     }
     return (ret_val);
}


bool TimerEx_getTimeout(TimerEx* self, timer_t *timer_id, int *sec, int *usec)
{
    int timerRef;
    bool ret_val = false;
    int temp;
    TimerEx_retTimerSet(self, timer_id, &timerRef);
    if (self->timers[timerRef].timer_end_sec > 0)
    {
        *sec = self->timers[timerRef].timer_end_sec - self->timers[timerRef].timer_start_sec;
        if (self->timers[timerRef].timer_start_usec > self->timers[timerRef].timer_end_usec)
        {
            temp=self->timers[timerRef].timer_start_usec - self->timers[timerRef].timer_end_usec;
            *usec = 1000000 - temp;
            *sec= *sec-1;
        }
        else
        {
            *usec = self->timers[timerRef].timer_end_usec - self->timers[timerRef].timer_start_usec;
        }
        ret_val = true;
    }
    return ret_val;
}

/*
    TimerEx_deleteTimer
*/
bool TimerEx_deleteTimer(TimerEx* self, timer_t *timer_id)
{
    int timerRef;
    int i;
    bool ret_val = false;
    for (i = 0; i < self->numTimer; i++)
    {
        if ( (void*)*timer_id == (void*)self->timers[i].timer_id )
        {    
            self->timers[i].timer_id = 0;
            self->timers[i].timer_act = TIMER_ONE_SHOT;
            self->timers[i].timer_bit = 0;
            self->timers[i].tv.tv_sec = 0;
            self->timers[i].tv.tv_usec = 0;
            if ((timer_delete(timer_id)) !=0)
            {
                TimerEx_retTimerSet(self, timer_id, &timerRef);
                fprintf(stderr,"----TimerEx: Timer:[%d] timer_delete failed:%d -(%s)\n",
                        timerRef, errno, strerror(errno));
               }
            else
            {
                ret_val = true;
            }
            break;
        }
    }
    return (ret_val);
}
