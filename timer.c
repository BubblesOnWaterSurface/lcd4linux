/* $Id: timer.c,v 1.4 2004/01/30 20:57:56 reinelt Exp $
 *
 * generic timer handling
 *
 * Copyright 2003,2004 Michael Reinelt <reinelt@eunet.at>
 * Copyright 2004 The LCD4Linux Team <lcd4linux-devel@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * $Log: timer.c,v $
 * Revision 1.4  2004/01/30 20:57:56  reinelt
 * HD44780 patch from Martin Hejl
 * dmalloc integrated
 *
 * Revision 1.3  2004/01/29 04:40:03  reinelt
 * every .c file includes "config.h" now
 *
 * Revision 1.2  2004/01/18 09:01:45  reinelt
 * /proc/stat parsing finished
 *
 * Revision 1.1  2004/01/13 08:18:20  reinelt
 * timer queues added
 * liblcd4linux deactivated turing transformation to new layout
 *
 */

/* 
 * exported functions:
 *
 * int timer_junk(void)
 *   Fixme: document me :-(
 *
 */


#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "debug.h"
#include "cfg.h"
#include "timer.h"

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


typedef struct TIMER {
  void (*callback)(void *data);
  void  *data;
  struct timeval when;
  int interval;
  int one_shot;
  int active;
} TIMER;


TIMER *Timers=NULL;
int   nTimers=0;


static void timer_inc (struct timeval *tv, int msec)
{
  tv->tv_sec  +=  msec                     / 1000;
  tv->tv_usec += (msec - 1000*(msec/1000)) * 1000;
  
  if (tv->tv_usec >= 1000000) {
    tv->tv_usec -= 1000000;
    tv->tv_sec++;
  }
}


int timer_add (void (*callback)(void *data), void *data, int interval, int one_shot)
{
  int i;
  struct timeval now;
  
  gettimeofday(&now, NULL);
  
  // find a free slot
  for (i=0; i<nTimers; i++) {
    if (Timers[i].active==0) break;
  }

  // none found, allocate a new slot
  if (i>=nTimers) {
    nTimers++;
    Timers=realloc(Timers, nTimers*sizeof(*Timers));
  }
  
  // fill slot
  Timers[i].callback = callback;
  Timers[i].data     = data;
  Timers[i].when     = now;
  Timers[i].interval = interval;
  Timers[i].one_shot = one_shot;
  Timers[i].active   = 1;

  // if one-shot timer, don't fire now
  if (one_shot) {
    timer_inc (&Timers[i].when, interval);
  }
  
  return 0;
}


int timer_process (struct timespec *delay)
{
  int i, flag, min;
  struct timeval now;
  
  // the current moment
  gettimeofday(&now, NULL);

  // sanity check
  if (nTimers==0) {
    error ("huh? not one single timer to process? dazed and confused...");
    return -1;
  }

  // process expired timers
  flag=0;
  for (i=0; i<nTimers; i++) {
    if (Timers[i].active == 0) continue;
    if (timercmp(&Timers[i].when, &now, <=)) {
      flag=1;
      // callback
      if (Timers[i].callback!=NULL) {
	Timers[i].callback(Timers[i].data);
      }
      // respawn or delete timer
      if (Timers[i].one_shot) {
	Timers[i].active=0;
      } else {
	Timers[i].when=now;
	timer_inc (&Timers[i].when, Timers[i].interval);
      }
    }
  }

  // find next timer
  flag=1;
  min=-1;
  for (i=0; i<nTimers; i++) {
    if (Timers[i].active == 0) continue;
    if (flag || timercmp(&Timers[i].when, &Timers[min].when, <)) {
      flag=0;
      min=i;
    }
  }
  
  if (min<0) {
    error ("huh? not one single timer left? dazed and confused...");
    return -1;
  }
    
  // delay until next timer event
  delay->tv_sec   = Timers[min].when.tv_sec  - now.tv_sec;
  delay->tv_nsec  = Timers[min].when.tv_usec - now.tv_usec;
  if (delay->tv_nsec<0) {
    delay->tv_sec--;
    delay->tv_nsec += 1000000;
  }
  // nanoseconds!!
  delay->tv_nsec *= 1000;

  return 0;
  
}
