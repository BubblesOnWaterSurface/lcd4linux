/* $Id: parport.h,v 1.4 2003/10/05 17:58:50 reinelt Exp $
 *
 * generic parallel port handling
 *
 * Copyright 2003 Michael Reinelt <reinelt@eunet.at>
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
 * $Log: parport.h,v $
 * Revision 1.4  2003/10/05 17:58:50  reinelt
 * libtool junk; copyright messages cleaned up
 *
 * Revision 1.3  2003/08/15 07:54:07  reinelt
 * HD44780 4 bit mode implemented
 *
 * Revision 1.2  2003/04/07 06:03:10  reinelt
 * further parallel port abstraction
 *
 * Revision 1.1  2003/04/04 06:02:03  reinelt
 * new parallel port abstraction scheme
 *
 */

#ifndef _PARPORT_H_
#define _PARPORT_H_

int parport_open (void);
int parport_close (void);
unsigned char parport_wire_ctrl (char *name, unsigned char *deflt);
unsigned char parport_wire_data (char *name, unsigned char *deflt);
void parport_direction (int direction);
void parport_control (unsigned char mask, unsigned char value);
void parport_toggle (unsigned char bit, int level, int delay);
void parport_data (unsigned char data);
unsigned char parport_read (void);
void parport_debug(void);

#endif
