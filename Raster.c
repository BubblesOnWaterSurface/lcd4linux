/* $Id: Raster.c,v 1.2 2000/03/24 11:36:56 reinelt Exp $
 *
 * driver for raster formats
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
 * $Log: Raster.c,v $
 * Revision 1.2  2000/03/24 11:36:56  reinelt
 *
 * new syntax for raster configuration
 * changed XRES and YRES to be configurable
 * PPM driver works nice
 *
 * Revision 1.1  2000/03/23 07:24:48  reinelt
 *
 * PPM driver up and running (but slow!)
 *
 */

/* 
 *
 * exported fuctions:
 *
 * struct DISPLAY Raster[]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "display.h"
#include "pixmap.h"

#define BARS ( BAR_L | BAR_R | BAR_U | BAR_D | BAR_H2 | BAR_V2 )

static DISPLAY Display;

static int pixel=-1;
static int pgap=0;
static int rgap=0;
static int cgap=0;
static int border=0;

static int foreground=0;
static int halfground=0;
static int background=0;

int Raster_flush (void)
{
  int xsize, ysize, row, col;
  unsigned char *buffer;
  unsigned char R[3], G[3], B[3];
  
  xsize=2*border+(Display.cols-1)*cgap+Display.cols*Display.xres*(pixel+pgap);
  ysize=2*border+(Display.rows-1)*rgap+Display.rows*Display.yres*(pixel+pgap);
  
  if ((buffer=malloc(xsize*ysize*sizeof(*buffer)))==NULL)
    return -1;

  memset (buffer, 0, xsize*ysize*sizeof(*buffer));
  
  for (row=0; row<Display.rows*Display.yres; row++) {
    int y=border+(row/Display.yres)*rgap+row*(pixel+pgap);
    for (col=0; col<Display.cols*Display.xres; col++) {
      int x=border+(col/Display.xres)*cgap+col*(pixel+pgap);
      int a, b;
      for (a=0; a<pixel; a++)
	for (b=0; b<pixel; b++)
	  buffer[y*xsize+x+a*xsize+b]=Pixmap[row*Display.cols*Display.xres+col]+1;
    }
  }
  
  printf ("P6\n%d %d\n255\n", xsize, ysize);
  
  R[0]=0xff&background>>16;
  G[0]=0xff&background>>8;
  B[0]=0xff&background;

  R[1]=0xff&halfground>>16;
  G[1]=0xff&halfground>>8;
  B[1]=0xff&halfground;

  R[2]=0xff&foreground>>16;
  G[2]=0xff&foreground>>8;
  B[2]=0xff&foreground;

  for (row=0; row<ysize; row++) {
    for (col=0; col<xsize; col++) {
      int i=buffer[row*xsize+col];
      printf("%c%c%c", R[i], G[i], B[i]);
    }
  }

  return 0;
}

int Raster_clear (void)
{
  if (pix_clear()!=0) 
    return -1;

  return 0;
}

int Raster_init (DISPLAY *Self)
{
  char *s;
  int rows=-1, cols=-1;
  int xres=1, yres=-1;
  
  if (sscanf(s=cfg_get("size")?:"20x4", "%dx%d", &cols, &rows)!=2 || rows<1 || cols<1) {
    fprintf (stderr, "Raster: bad size '%s'\n", s);
    return -1;
  }

  if (sscanf(s=cfg_get("font")?:"5x8", "%dx%d", &xres, &yres)!=2 || xres<5 || yres<7) {
    fprintf (stderr, "Raster: bad font '%s'\n", s);
    return -1;
  }

  if (sscanf(s=cfg_get("pixel")?:"4+1", "%d+%d", &pixel, &pgap)!=2 || pixel<1 || pgap<0) {
    fprintf (stderr, "Raster: bad pixel '%s'\n", s);
    return -1;
  }

  if (sscanf(s=cfg_get("gap")?:"3x3", "%dx%d", &rgap, &cgap)!=2 || rgap<0 || cgap<0) {
    fprintf (stderr, "Raster: bad gap '%s'\n", s);
    return -1;
  }

  border=atoi(cfg_get("border")?:"0");

  foreground=strtol(cfg_get("foreground")?:"000000", NULL, 16);
  halfground=strtol(cfg_get("halfground")?:"ffffff", NULL, 16);
  background=strtol(cfg_get("background")?:"ffffff", NULL, 16);

  if (pix_init (rows, cols, xres, yres)!=0) {
    fprintf (stderr, "Raster: pix_init(%d, %d, %d, %d) failed\n", rows, cols, xres, yres);
    return -1;
  }

  Self->rows=rows;
  Self->cols=cols;
  Self->xres=xres;
  Self->yres=yres;
  Display=*Self;

  pix_clear();
  return 0;
}

int Raster_put (int row, int col, char *text)
{
  return pix_put (row, col, text);
}

int Raster_bar (int type, int row, int col, int max, int len1, int len2)
{
  return pix_bar (type, row, col, max, len1, len2);
}


DISPLAY Raster[] = {
  { "PPM", 0, 0, 0, 0, BARS, Raster_init, Raster_clear, Raster_put, Raster_bar, Raster_flush },
  { "" }
};
