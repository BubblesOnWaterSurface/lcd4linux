/* $Id: isdn.c,v 1.13 2004/01/29 04:40:02 reinelt Exp $
 *
 * ISDN specific functions
 *
 * Copyright 1999, 2000 Michael Reinelt <reinelt@eunet.at>
 *
 * This file is part of LCD4Linux.
 *
 * LCD4Linux is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * LCD4Linux is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * $Log: isdn.c,v $
 * Revision 1.13  2004/01/29 04:40:02  reinelt
 * every .c file includes "config.h" now
 *
 * Revision 1.12  2003/11/24 11:34:54  reinelt
 *
 * 'Fixed' Rows which do not scroll by Lars Kempe
 * temporary workaround for debian kernel-header bug
 *
 * Revision 1.11  2003/10/05 17:58:50  reinelt
 * libtool junk; copyright messages cleaned up
 *
 * Revision 1.10  2003/06/21 05:46:18  reinelt
 * DVB client integrated
 *
 * Revision 1.9  2001/05/27 07:19:28  reinelt
 *
 * fixed a warning in pixmap.c
 * temporarily fixed a bug in isdn.c (ISDN_MAX_CHANNELS is no longer defined?)
 * fixed a bug in configure.in (--with-drivers=xyz did not work)
 *
 * Revision 1.8  2000/08/10 18:42:20  reinelt
 *
 * fixed some bugs with the new syslog code
 *
 * Revision 1.7  2000/08/10 09:44:09  reinelt
 *
 * new debugging scheme: error(), info(), debug()
 * uses syslog if in daemon mode
 *
 * Revision 1.6  2000/04/15 11:56:35  reinelt
 *
 * more debug messages
 *
 * Revision 1.5  2000/03/13 15:58:24  reinelt
 *
 * release 0.9
 * moved row parsing to parser.c
 * all basic work finished
 *
 * Revision 1.4  2000/03/10 17:36:02  reinelt
 *
 * first unstable but running release
 *
 * Revision 1.3  2000/03/07 11:01:34  reinelt
 *
 * system.c cleanup
 *
 * Revision 1.2  2000/03/06 06:04:06  reinelt
 *
 * minor cleanups
 *
 */

/* 
 * exported functions:
 *
 * Isdn (int *rx, int *tx, int *usage)
 *   returns 0 if ok, -1 if error
 *   sets *usage to all channels USAGE or'ed together
 *   sets received/transmitted bytes in *rx, *tx
 *
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

// Fixme: my debian has some bugs with kernel headers
#if 0
#include <linux/isdn.h>
#else
#define IIOCGETCPS  _IO('I',21)
#endif

// Fixme: ISDN_MAX_CHANNELS seems undefined in recent kernels
#ifndef ISDN_MAX_CHANNELS
#define ISDN_MAX_CHANNELS 64
#endif

#include "debug.h"
#include "isdn.h"
#include "filter.h"

typedef struct {
  unsigned long in;
  unsigned long out;
} CPS;


static int Usage (void)
{
  static int fd=0;
  char buffer[4096], *p;
  int i, usage;

  if (fd==-1) return 0;

  fd=open ("/dev/isdninfo", O_RDONLY | O_NDELAY);
  if (fd==-1) {
    error ("open(/dev/isdninfo) failed: %s", strerror(errno));
    return 0;
  }
  
  if (read (fd, buffer, sizeof(buffer))==-1) {
    error ("read(/dev/isdninfo) failed: %s", strerror(errno));
    fd=-1;
    return 0;
  }

  if (close(fd)==-1) {
    error ("close(/dev/isdninfo) failed: %s", strerror(errno));
    fd=-1;
    return 0;
  }

  p=strstr(buffer, "usage:");
  if (p==NULL) {
    error ("parse(/dev/isdninfo) failed: no usage line");
    fd=-1;
    return 0;
  }
  p+=6;

  usage=0;
  for (i=0; i<ISDN_MAX_CHANNELS; i++) {
    usage|=strtol(p, &p, 10);
  }
  return usage;
}

int Isdn (int *rx, int *tx, int *usage)
{
  static int fd=-2;
  CPS cps[ISDN_MAX_CHANNELS];
  double cps_i, cps_o;
  int i;

  *usage=0;
  *rx=0;
  *tx=0;

  if (fd==-1) return -1;
  
  if (fd==-2) {
    fd = open("/dev/isdninfo", O_RDONLY | O_NDELAY);
    if (fd==-1) {
      error ("open(/dev/isdninfo) failed: %s", strerror(errno));
      return -1;
    }
    debug ("open (/dev/isdninfo)=%d", fd);
  }

  if (ioctl(fd, IIOCGETCPS, &cps)) {
    error("ioctl(IIOCGETCPS) failed: %s", strerror(errno));
    fd=-1;
    return -1;
  }

  cps_i=0;
  cps_o=0;
  for (i=0; i<ISDN_MAX_CHANNELS; i++) {
    cps_i+=cps[i].in;
    cps_o+=cps[i].out;
  }

  *rx=(int)smooth("isdn_rx", 1000, cps_i);
  *tx=(int)smooth("isdn_tx", 1000, cps_o);
  *usage=Usage();

  return 0;
}

