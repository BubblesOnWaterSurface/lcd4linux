/* $Id$
 * $URL$
 *
 * dynamic properties
 *
 * Copyright (C) 2006 Michael Reinelt <reinelt@eunet.at>
 * Copyright (C) 2006 The LCD4Linux Team <lcd4linux-devel@users.sourceforge.net>
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
 * $Log: property.h,v $
 * Revision 1.3  2006/08/14 05:54:04  reinelt
 * minor warnings fixed, CFLAGS changed (no-strict-aliasing)
 *
 * Revision 1.2  2006/08/13 11:38:20  reinelt
 * text widget uses dynamic properties
 *
 * Revision 1.1  2006/08/13 09:53:10  reinelt
 * dynamic properties added (used by 'style' of text widget)
 *
 */


#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include "evaluator.h"


typedef struct {
    char *name;
    char *expression;
    void *compiled;
    RESULT result;
} PROPERTY;


void property_load(const char *section, const char *name, const char *defval, PROPERTY * prop);
int property_eval(PROPERTY * prop);
double P2N(PROPERTY * prop);
char *P2S(PROPERTY * prop);
void property_free(PROPERTY * prop);

#endif
