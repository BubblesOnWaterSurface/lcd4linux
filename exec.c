/* $Id: exec.c,v 1.3 2001/03/08 15:25:38 ltoetsch Exp $
 *
 * exec ('x*') functions
 *
 * Copyright 2001 by Leopold T�tsch (lt@toetsch.at)
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
 * $Log: exec.c,v $
 * Revision 1.3  2001/03/08 15:25:38  ltoetsch
 * improved exec
 *
 * Revision 1.2  2001/03/08 08:39:54  reinelt
 *
 * fixed two typos
 *
 * Revision 1.1  2001/03/07 18:10:21  ltoetsch
 * added e(x)ec commands
 *
 *
 * This implements the x1 .. x9 commands
 * config options:
 *   x1 .. x9      command to execute
 *   Tick_x1 ... 9 delay in ticks
 *   Delay_x1 .. 9 delay in seconds
 *   Scale_x1 .. 9 scale for bars
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#define IN_EXEC
#include "exec.h"
#include "debug.h"
#include "cfg.h"


int Exec(int index, char buff[EXEC_TXT_LEN], double *val)
{
  static time_t now[EXECS+1];
  static int errs[EXECS+1];
  static int ticks[EXECS+1];
  char *command, *p;
  char xn[20];
  char env[EXEC_TXT_LEN];
  FILE *pipe;
  size_t len;
  int i;

  if (index < 1 || index > EXECS)
    return -1; 
  if (errs[index])
    return -1;
  
  /* first time ? */
  if (now[index] != 0) {
    /* delay in Ticks ? */
    sprintf(xn, "Tick_x%d", index);
    p = cfg_get(xn);
    if (p && *p) {
      if (ticks[index]++ % atoi(p) != 0)
        return 0;
    }
    else {
      sprintf(xn, "Delay_x%d", index);
      /* delay in Delay_x* sec ? */
      if (time(NULL) <= now[index] + atoi(cfg_get(xn)?:"1"))
        return 0;
    }
  }
  time(&now[index]); 
  *val = -1;
  
  sprintf(xn, "x%d", index);
  command = cfg_get(xn);
  debug("%s:'%s'",xn,command);
					    
  if (!command || !*command) {
    error("Empty command for 'x%d'", index);
    errs[index]++;
    return -1;
  }
  for (i = 1; i < index; i++) {
    sprintf(env, "X%d=%.*s", i, EXEC_TXT_LEN-10, exec[i].s);
    putenv(env);
  }
  putenv("PATH=/usr/local/bin:/usr/bin:/bin");
  pipe = popen(command, "r");
  if (pipe == NULL) {
    error("Couldn't run pipe '%s':\n%s", command, strerror(errno));
    errs[index]++;
    return -1;
  }
  len = fread(buff, 1, EXEC_TXT_LEN-1,  pipe);
  if (len <= 0) {
    pclose(pipe);
    error("Couldn't fread from pipe '%s', len=%d", command, len);
    errs[index]++;
    *buff = '\0';
    return -1;
  }
  pclose(pipe);
  buff[len] = '\0';
  for (p = buff ; *p && isspace(*p); p++)
    ;
  if (isdigit(*p)) {
    *val = atof(p);
    sprintf(xn, "Scale_x%d", index);
    *val /= atoi(cfg_get(xn)?:"100")?:100;
  }
  return 0;
}

