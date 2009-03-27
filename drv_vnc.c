/* $Id$
 * $URL$
 *
 * Libvncserver driver
 * 
 * Copyright (C) 2009 Michael Vogt <michu@neophob.com>
 * Modified from sample code by:
 * Copyright (C) 2005 Michael Reinelt <michael@reinelt.co.at>
 * Copyright (C) 2005, 2006, 2007, 2008, 2009 The LCD4Linux Team <lcd4linux-devel@users.sourceforge.net>
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
 */

/* 
 *
 * exported fuctions:
 *
 * struct DRIVER drv_vnc
 *
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <rfb/rfb.h>

/* struct timeval */
#include <sys/time.h>

#include "debug.h"
#include "cfg.h"
#include "qprintf.h"
#include "udelay.h"
#include "plugin.h"
#include "widget.h"
#include "widget_text.h"
#include "widget_icon.h"
#include "widget_bar.h"
#include "widget_keypad.h"
#include "drv.h"
#include "drv_generic_graphic.h"
#include "drv_generic_keypad.h"

//todo: fps limiter
//      key widget

/* 15 frames per second (if we can) */
#define PICTURE_TIMEOUT (1.0/15.0)

static char Name[] = "VNC";

static int xres = 320;		/* screen settings */
static int yres = 200;
static int BPP = 4;
static int max_clients = 2;	/* max connected clients */
static int osd_showtime = 2000;	/* time to display the osd in ms */
static int buttons = 2;		/* number of keypad buttons */
static int buttonsize = 50;	/* size of keypad buttons */
static int keypadxofs = 0;
static int keypadyofs = 0;
static int port = 5900;

static rfbScreenInfoPtr server;	/* vnc device */
static struct timeval osd_timestamp;
static int show_keypad_osd = 0;	/* is the osd active? */
static int clientCount = 0;	/* currently connected clients */
static int mouse_x = 0;
static int mouse_y = 0;

/* draws a simple rect, used to display keypad */
int draw_rect(int x, int y, int size, unsigned char col, unsigned char *buffer)
{
    int ofs, i, ret;
    unsigned char colr, colg;
    colr = colg = col;
    ret = 0;

    if (mouse_x > x && mouse_x < (x + size))
	if (mouse_y > y && mouse_y < (y + size)) {
	    colr = 128;
	    colg = 255;
	    ret = 1;
	}

    for (i = x; i < x + size; i++) {
	ofs = (i + xres * y) * BPP;
	buffer[ofs++] = colr;
	buffer[ofs++] = colg;
	buffer[ofs++] = col;

	ofs = (i + xres * (y + size)) * BPP;
	buffer[ofs++] = colr;
	buffer[ofs++] = colg;
	buffer[ofs++] = col;
    }

    for (i = y; i <= y + size; i++) {
	ofs = (i * xres + x) * BPP;
	buffer[ofs++] = colr;
	buffer[ofs++] = colg;
	buffer[ofs++] = col;

	ofs = (i * xres + x + size) * BPP;
	buffer[ofs++] = colr;
	buffer[ofs++] = colg;
	buffer[ofs++] = col;
    }
    return ret;
}

void display_keypad()
{
    int i, rectx, recty;
    int gap = 10;
    int ret;
    for (i = 0; i < buttons; i++) {
	rectx = keypadxofs + (i * (buttonsize + gap));
	recty = keypadyofs /*+ (i*(buttonsize+gap)) */ ;
	ret = draw_rect(rectx, recty, buttonsize, 0, server->frameBuffer);

	if (ret == 1) {
	    drv_generic_keypad_press(i + 1);
	    //debug("mouse in keypad nr %d", i);
	}
    }
}

/* called if a vnc client disconnects */
static void clientgone(rfbClientPtr cl)
{
    if (clientCount > 0)
	clientCount--;
    debug("%d clients connecten", clientCount);
}

/* called if a vnc client connect */
static enum rfbNewClientAction hook_newclient(rfbClientPtr cl)
{
    if (clientCount < max_clients) {
	clientCount++;
	cl->clientGoneHook = clientgone;
	debug("%d clients connected", clientCount);
	return RFB_CLIENT_ACCEPT;
    } else {
	info("client refused due max. client connections (%d)", clientCount);
	return RFB_CLIENT_REFUSE;
    }
}

/* handle mouse action */
static void hook_mouseaction(int buttonMask, int x, int y, rfbClientPtr cl)
{
    if (x >= 0 && y >= 0 && x < xres && y < yres) {
	/* we check only, if the left mousebutton is pressed */
	if (buttonMask == 1) {
	    gettimeofday(&osd_timestamp, NULL);
	    mouse_x = x;
	    mouse_y = y;

	    if (show_keypad_osd == 0) {
		/* no osd until yet, activate osd keypad ... */
		show_keypad_osd = 1;
	    }
	}
    }

    rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
}

static int drv_vnc_keypad(const int num)
{
    int val = WIDGET_KEY_PRESSED;

    switch (num) {
    case 1:
	val += WIDGET_KEY_UP;
	break;
    case 2:
	val += WIDGET_KEY_DOWN;
	break;
    case 3:
	val += WIDGET_KEY_LEFT;
	break;
    case 4:
	val += WIDGET_KEY_RIGHT;
	break;
    case 5:
	val += WIDGET_KEY_CONFIRM;
	break;
    case 6:
	val += WIDGET_KEY_CANCEL;
	break;
    default:
	error("%s: unknown keypad value %d", Name, num);
    }

    return val;
}

/* init the driver, read config */
static int drv_vnc_open(const char *Section)
{
    if (cfg_number(Section, "Xres", 320, 32, 2048, &xres) < 1) {
	info("[DRV_VNC] no '%s.Xres' entry from %s using default %d", Section, cfg_source(), xres);
    }
    if (cfg_number(Section, "Yres", 200, 32, 2048, &yres) < 1) {
	info("[DRV_VNC] no '%s.Yres' entry from %s using default %d", Section, cfg_source(), yres);
    }
    if (cfg_number(Section, "Bpp", 4, 1, 4, &BPP) < 1) {
	info("[DRV_VNC] no '%s.Bpp' entry from %s using default %d", Section, cfg_source(), BPP);
    }
    if (cfg_number(Section, "Maxclients", 2, 1, 64, &max_clients) < 1) {
	info("[DRV_VNC] no '%s.Maxclients' entry from %s using default %d", Section, cfg_source(), max_clients);
    }
    if (cfg_number(Section, "Osd_showtime", 2000, 500, 60000, &osd_showtime) < 1) {
	info("[DRV_VNC] no '%s.Osd_showtime' entry from %s using default %d", Section, cfg_source(), osd_showtime);
    }
    if (cfg_number(Section, "Buttons", 2, 0, 6, &buttons) < 1) {
	info("[DRV_VNC] no '%s.Buttons' entry from %s using default %d", Section, cfg_source(), buttons);
    }
    if (cfg_number(Section, "Buttonsize", 50, 8, 256, &buttonsize) < 1) {
	info("[DRV_VNC] no '%s.Buttonsize' entry from %s using default %d", Section, cfg_source(), buttonsize);
    }
    if (cfg_number(Section, "Keypadxofs", 0, 0, 4096, &keypadxofs) < 1) {
	info("[DRV_VNC] no '%s.Keypadxofs' entry from %s using default %d", Section, cfg_source(), keypadxofs);
    }
    if (cfg_number(Section, "Keypadyofs", 0, 0, 4096, &keypadyofs) < 1) {
	info("[DRV_VNC] no '%s.Keypadyofs' entry from %s using default %d", Section, cfg_source(), keypadyofs);
    }
    if (cfg_number(Section, "Port", 5900, 1, 65535, &port) < 1) {
	info("[DRV_VNC] no '%s.Port' entry from %s using default %d", Section, cfg_source(), port);
    }

    return 0;
}

/* shutdown driver, release allocated stuff */
static int drv_vnc_close(void)
{
    rfbShutdownServer(server, TRUE);
    free(server->frameBuffer);
    return 0;
}


/* actual blitting method */
static void drv_vnc_blit_it(const int row, const int col, const int height, const int width, unsigned char *buffer)
{
    int r, c, ofs;
    RGBA p;

    for (r = row; r < row + height; r++) {
	for (c = col; c < col + width; c++) {
	    p = drv_generic_graphic_rgb(r, c);
	    ofs = (r * xres + c) * BPP;
	    buffer[ofs++] = p.R;
	    buffer[ofs++] = p.G;
	    buffer[ofs++] = p.B;
	    buffer[ofs] = 255;
	}
    }

    /* display osd keypad */
    if (show_keypad_osd == 1) {
	display_keypad();

	/* check if the osd should be disabled after the waittime */
	struct timeval now;
	gettimeofday(&now, NULL);
	int timedelta = (now.tv_sec - osd_timestamp.tv_sec) * 1000 + (now.tv_usec - osd_timestamp.tv_usec) / 1000;

	if (timedelta > osd_showtime) {
	    show_keypad_osd = 0;
	}
    }
}

static void drv_vnc_blit(const int row, const int col, const int height, const int width)
{

    if (rfbIsActive(server)) {
	drv_vnc_blit_it(row, col, height, width, (unsigned char *) server->frameBuffer);

	if (clientCount > 0) {
	    rfbMarkRectAsModified(server, 0, 0, xres, yres);
	}
	rfbProcessEvents(server, server->deferUpdateTime * 500);
    }
}

/* start graphic display */
static int drv_vnc_start(const char *section)
{
    char *s;

    s = cfg_get(section, "Font", "6x8");
    if (s == NULL || *s == '\0') {
	error("%s: no '%s.Font' entry from %s", Name, section, cfg_source());
	return -1;
    }

    XRES = -1;
    YRES = -1;
    if (sscanf(s, "%dx%d", &XRES, &YRES) != 2 || XRES < 1 || YRES < 1) {
	error("%s: bad Font '%s' from %s", Name, s, cfg_source());
	return -1;
    }

    /* Fixme: provider other fonts someday... */
    if (XRES != 6 && YRES != 8) {
	error("%s: bad Font '%s' from %s (only 6x8 at the moment)", Name, s, cfg_source());
	return -1;
    }

    /* open communication with the display */
    if (drv_vnc_open(section) < 0) {
	return -1;
    }

    /* you surely want to allocate a framebuffer or something... */
    server = rfbGetScreen(0, NULL, xres, yres, 8, 3, BPP);
    server->desktopName = "LCD4Linux VNC Driver";
    server->frameBuffer = (char *) malloc(xres * yres * BPP);
    server->alwaysShared = (1 == 1);
    server->port = port;
    server->ptrAddEvent = hook_mouseaction;
    server->newClientHook = hook_newclient;

    /* Initialize the server */
    rfbInitServer(server);

    /* set width/height */
    DROWS = yres;
    DCOLS = xres;

    return 0;
}

/****************************************/
/***        widget callbacks          ***/
/****************************************/


/* using drv_generic_text_draw(W) */
/* using drv_generic_text_icon_draw(W) */
/* using drv_generic_text_bar_draw(W) */
/* using drv_generic_gpio_draw(W) */


/****************************************/
/***        exported functions        ***/
/****************************************/


/* list models */
int drv_vnc_list(void)
{
    printf("vnc server");
    return 0;
}


/* initialize driver & display */
int drv_vnc_init(const char *section, const int quiet)
{
    int ret;

    /* real worker functions */
    drv_generic_graphic_real_blit = drv_vnc_blit;
    drv_generic_keypad_real_press = drv_vnc_keypad;

    /* start display */
    if ((ret = drv_vnc_start(section)) != 0)
	return ret;

    /* initialize generic graphic driver */
    if ((ret = drv_generic_graphic_init(section, Name)) != 0)
	return ret;

    /* initialize generic key pad driver */
    if ((ret = drv_generic_keypad_init(section, Name)) != 0)
	return ret;

    if (!quiet) {
	char buffer[40];
	qprintf(buffer, sizeof(buffer), "%s %dx%d", Name, DCOLS, DROWS);
	if (drv_generic_graphic_greet(buffer, NULL)) {
	    sleep(3);
	    drv_generic_graphic_clear();
	}
    }

    return 0;
}


/* close driver & display */
int drv_vnc_quit(const int quiet)
{
    info("%s: shutting down.", Name);

    /* clear display */
    drv_generic_graphic_clear();

    /* say goodbye... */
    if (!quiet) {
	drv_generic_graphic_greet("goodbye!", NULL);
    }

    drv_generic_graphic_quit();
    drv_generic_keypad_quit();

    debug("closing connection");
    drv_vnc_close();

    return (0);
}


DRIVER drv_vnc = {
    .name = Name,
    .list = drv_vnc_list,
    .init = drv_vnc_init,
    .quit = drv_vnc_quit,
};
