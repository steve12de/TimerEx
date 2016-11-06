//****************************************************************************
// File:             timerEx.h
// Revision:	     3
// Author   	     Stephen Beko
// Purpose	         Library Function Headers for Application Timers
//
// Use:
//    typedef struct test_type{
//        TimerEx *timex;        // see structures below
//        timer_t timer1; 
//        } TEST; 
//
//****************************************************************************
#ifndef TIMEREX_H
#define TIMEREX_H

#include <signal.h>
#include <time.h>
#include <stdbool.h>

// Max number of Timers is number of bits in unsigned long, is 32
#define TIMER_MAX    10

typedef enum
{
    TIMER_ONE_SHOT,
    TIMER_RESTART
} timer_action;

typedef struct tm_timers{
    timer_t timer_id;
    timer_action timer_act;
    int timer_bit;
    struct timeval tv;
    // For a time measurement
    long timer_start_sec;
    long timer_start_usec;
    long timer_end_sec;
    long timer_end_usec;
} tm_timers;

typedef struct TimerEx
{
    int numTimer;
    tm_timers timers[TIMER_MAX];
} TimerEx;

void TimerEx_construct(TimerEx* self);
int  TimerEx_makeTimer(char*, timer_t*, unsigned int, unsigned int);
void TimerEx_put_timer(TimerEx* self, timer_t *timer_id, timer_action act);
bool TimerEx_retTimerSet(TimerEx* self, timer_t *timer_id, int *timerRef);
bool TimerEx_stoptimer(TimerEx* self, timer_t *timer_id);
bool TimerEx_starttimer(TimerEx* self, timer_t *timer_id, unsigned int expireMS, unsigned int intervalMS );
bool TimerEx_deleteTimer(TimerEx* self, timer_t *timer_id);
bool TimerEx_getTime(TimerEx* self, timer_t *timer_id, int *sec, int *usec);
#endif  //TIMEREX_H
