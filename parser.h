/* $Id: parser.h,v 1.11 2001/03/14 13:19:29 ltoetsch Exp $
 *
 * row definition parser
 *
 * Copyright 1999, 2000 by Michael Reinelt (reinelt@eunet.at)
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
 * $Log: parser.h,v $
 * Revision 1.11  2001/03/14 13:19:29  ltoetsch
 * Added pop3/imap4 mail support
 *
 * Revision 1.10  2001/03/07 18:10:21  ltoetsch
 * added e(x)ec commands
 *
 * Revision 1.9  2001/03/02 10:18:03  ltoetsch
 * added /proc/apm battery stat
 *
 * Revision 1.8  2001/02/19 00:15:46  reinelt
 *
 * integrated mail and seti client
 * major rewrite of parser and tokenizer to support double-byte tokens
 *
 * Revision 1.7  2001/02/16 08:23:09  reinelt
 *
 * new token 'ic' (ISDN connected) by Carsten Nau <info@cnau.de>
 *
 * Revision 1.6  2001/02/14 07:40:16  reinelt
 *
 * first (incomplete) GPO implementation
 *
 * Revision 1.5  2000/05/21 06:20:35  reinelt
 *
 * added ppp throughput
 * token is '%t[iomt]' at the moment, but this will change in the near future
 *
 * Revision 1.4  2000/03/19 08:41:28  reinelt
 *
 * documentation available! README, README.MatrixOrbital, README.Drivers
 * added Skeleton.c as a starting point for new drivers
 *
 * Revision 1.3  2000/03/18 10:31:06  reinelt
 *
 * added sensor handling (for temperature etc.)
 * made data collecting happen only if data is used
 * (reading /proc/meminfo takes a lot of CPU!)
 * released lcd4linux-0.92
 *
 * Revision 1.2  2000/03/17 09:21:42  reinelt
 *
 * various memory statistics added
 *
 * Revision 1.1  2000/03/13 15:58:24  reinelt
 *
 * release 0.9
 * moved row parsing to parser.c
 * all basic work finished
 *
 */

#ifndef _PARSER_H_
#define _PARSER_H_

typedef enum {
  T_PERCENT=128, T_DOLLAR,
  T_OS, T_RELEASE, T_CPU, T_RAM,
  T_MEM_TOTAL, T_MEM_USED, T_MEM_FREE, T_MEM_SHARED, T_MEM_BUFFER, T_MEM_CACHE, T_MEM_AVAIL, 
  T_LOAD_1, T_LOAD_2, T_LOAD_3, T_OVERLOAD, 
  T_CPU_USER, T_CPU_NICE, T_CPU_SYSTEM, T_CPU_BUSY, T_CPU_IDLE,
  T_DISK_READ, T_DISK_WRITE, T_DISK_TOTAL, T_DISK_MAX,
  T_ETH_RX, T_ETH_TX, T_ETH_TOTAL, T_ETH_MAX,
  T_PPP_RX, T_PPP_TX, T_PPP_TOTAL, T_PPP_MAX,
  T_ISDN_USED, T_ISDN_IN, T_ISDN_OUT, T_ISDN_TOTAL, T_ISDN_MAX,
  T_SETI_PRC, T_SETI_CPU,
  T_BATT_PERC, T_BATT_STAT, T_BATT_DUR,
  T_EXTENDED,
  T_MAIL, T_MAIL_UNSEEN,
  T_SENSOR,
  T_EXEC  
} TOKEN;

typedef enum {
  C_GENERIC, C_MEM, C_LOAD, C_CPU, C_DISK, C_ETH, C_PPP, C_ISDN, C_SETI, 
  C_BATT,  
  C_MAIL, C_SENSOR, C_EXEC
} CLASS;

char *parse_row (char *string, int supported_bars, int usage[]);
int   parse_gpo (char *string, int usage[]);

#endif
