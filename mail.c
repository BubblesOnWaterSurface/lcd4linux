/* $Id: mail.c,v 1.3 2001/02/21 04:48:13 reinelt Exp $
 *
 * email specific functions
 *
 * Copyright 2001 by Axel Ehnert <Axel@Ehnert.net>
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
 * $Log: mail.c,v $
 * Revision 1.3  2001/02/21 04:48:13  reinelt
 *
 * big mailbox patch from Axel Ehnert
 * thanks to herp for his idea to check mtime of mailbox
 *
 * Revision 1.2  2001/02/19 00:15:46  reinelt
 *
 * integrated mail and seti client
 * major rewrite of parser and tokenizer to support double-byte tokens
 *
 * Revision 1.1  2001/02/18 22:11:34  reinelt
 * *** empty log message ***
 *
 */

/* 
 * exported functions:
 *
 * Mail (int index, int *num)
 *   returns 0 if ok, -1 if error
 *   sets num to number of emails in mailbox #index
 *
 */

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "cfg.h"
#include "debug.h"
#include "mail.h"

int Mail (int index, int *num)
{
  FILE *fstr;
  char buffer[32];
  static int cfgmbx[MAILBOXES+1]={[0 ... MAILBOXES]=TRUE,}; // Mailbox #index configured?
  static time_t mbxlt[MAILBOXES+1]={[0 ... MAILBOXES]=0,};  // mtime of Mailbox #index
  static int mbxnum[MAILBOXES+1]={[0 ... MAILBOXES]=0,};    // Last calculated # of mails
  static time_t now[MAILBOXES+1]={[0 ... MAILBOXES]=0,};    // Last call to procedure at 
                                                            // for Mailbox #index
  char *fnp1;
  int v1=0;
  int last_line_blank1;                   // Was the last line blank?
  struct stat fst;
  int rc;

  char *txt;
  char txt1[100];

  if (index<1 || index>MAILBOXES) return -1;

  if (time(NULL)==now[index]) return 0;   // More then 1 second after last check
  time(&now[index]);                      // for Mailbox #index
  /*
    Build the filename from the config
  */
  snprintf(buffer, 32, "Mailbox%d", index);
  fnp1=cfg_get(buffer);
  if (fnp1==NULL || *fnp1=='\0') {
    cfgmbx[index]=FALSE;                  // There is now entry for Mailbox #index
  }
  v1=mbxnum[index];
  /*
    Open the file
  */
  if (cfgmbx[index]==TRUE) {
  /*
    Check the last touch of mailbox. Changed?
  */
    rc=stat(fnp1, &fst);
    if ( rc != 0 ) {
      error ("Error getting stat of Mailbox%d", index );
      return (-1);
    }
    if ( mbxlt[index] != fst.st_mtime ) {
      mbxlt[index]=fst.st_mtime;

      fstr=fopen(fnp1,"r");

      if (fstr != NULL) {
        txt=&txt1[0];
        last_line_blank1=TRUE;
        v1=0;

        while ( ( fgets ( txt1, 100, fstr ) ) != NULL ) {
          txt1[strlen(txt1)-1]='\0';                 // cut the newline
       	  /*
	    Is there a "From ..." line. Count only, if a blank line was directly before this
	  */
          if ( strncmp (txt1, "From ", 5 ) == 0 ) {
            if ( last_line_blank1 == TRUE ) {
              v1++;
              debug ("mailbox%d found mail %d",index, v1);
              last_line_blank1 = FALSE;
            }
          }
          if ( strlen (txt1) == 0 ) {
  	    last_line_blank1 = TRUE;
          }
          else {
  	    last_line_blank1 = FALSE;
          }
        }
        fclose (fstr);
      }
    }
  }
  mbxnum[index]=v1;
  *num=v1;
  return (0);
}

