/* $Id: isdn.h,v 1.6 2003/10/05 17:58:50 reinelt Exp $
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
 * $Log: isdn.h,v $
 * Revision 1.6  2003/10/05 17:58:50  reinelt
 * libtool junk; copyright messages cleaned up
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

#ifndef _ISDN_H_
#define _ISDN_H_

int Isdn (int *rx, int *tx, int *usage);

#endif
