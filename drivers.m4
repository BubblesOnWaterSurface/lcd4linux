dnl LCD4Linux Drivers conf part
dnl
dnl Copyright (C) 1999, 2000, 2001, 2002, 2003 Michael Reinelt <reinelt@eunet.at>
dnl Copyright (C) 2004 The LCD4Linux Team <lcd4linux-devel@users.sourceforge.net>
dnl
dnl This file is part of LCD4Linux.
dnl
dnl LCD4Linux is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl LCD4Linux is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

AC_MSG_CHECKING([which drivers to compile])
AC_ARG_WITH(
  drivers, 
  [  --with-drivers=<list>   compile driver for displays in <list>,]
  [                        drivers may be separated with commas,]	
  [                        'all' (default) compiles all available drivers,]	
  [                        drivers may be excluded with 'all,!<driver>',]	
  [                        (try 'all,\!<driver>' if your shell complains...)]	
  [                        possible drivers are:]	
  [                        BeckmannEgle, BWCT, CrystalFontz, Curses, Cwlinux,]
  [                        HD44780, LCDLinux, LCDTerm, M50530, MatrixOrbital,]
  [                        MilfordInstruments, NULL, PNG, PPM, RouterBoard,]
  [                        SimpleLCD, T6963, USBLCD, X11],
  drivers=$withval, 
  drivers=all
)

drivers=`echo $drivers|sed 's/,/ /g'`

for driver in $drivers; do

   case $driver in 
      !*) 
         val="no"
         driver=`echo $driver|cut -c 2-`
         ;;
       *) 
         val="yes"
         ;;
   esac
	
   case "$driver" in
      all)
         BECKMANNEGLE="yes"
         BWCT="yes"
         CRYSTALFONTZ="yes"
         CURSES="yes"
         CWLINUX="yes"
         HD44780="yes"
	 LCDLINUX="yes"
         LCDTERM="yes"
         M50530="yes"
         MATRIXORBITAL="yes"
         MILINST="yes"
         NULL="yes" 
         PNG="yes"
         PPM="yes"
	 ROUTERBOARD="yes"
         T6963="yes"
         USBLCD="yes"
         X11="yes"
         SIMPLELCD="yes"
         ;;
      BeckmannEgle)
         BECKMANNEGLE=$val
         ;;
      BWCT)
         BWCT=$val
         ;;
      CrystalFontz)
         CRYSTALFONTZ=$val
         ;;
      Curses)
         CURSES=$val
         ;;
      Cwlinux)
         CWLINUX=$val
         ;;
      HD44780)
         HD44780=$val
	 ;;
      LCDLINUX)
         LCDLINUX=$val
	 ;;
      LCDTerm)
         LCDTERM=$val
	 ;;
      M50530)
         M50530=$val
         ;;
      MatrixOrbital)
         MATRIXORBITAL=$val
         ;;
      MilfordInstruments)
         MILINST=$val
         ;;
      NULL)
         NULL=$val;
         ;;
      PNG)
         PNG=$val
         ;;
      PPM)
         PPM=$val
         ;;
      RouterBoard)
         ROUTERBOARD=$val
         ;;
      SimpleLCD)
         SIMPLELCD=$val
         ;;
      T6963)
         T6963=$val
         ;;
      USBLCD)
         USBLCD=$val
         ;;
      X11)
         X11=$val
         ;;
      *) 	
         AC_MSG_ERROR([Unknown driver '$driver'])
         ;;
   esac
done

AC_MSG_RESULT([done])

PARPORT="no"
SERIAL="no"
TEXT="no"
GRAPHIC="no"
IMAGE="no"

if test "$BECKMANNEGLE" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_BeckmannEgle.o"
   AC_DEFINE(WITH_BECKMANNEGLE,1,[Beckmann&Egle driver])
fi

if test "$BWCT" = "yes"; then
   if test "$has_usb" = "true"; then
      TEXT="yes"
      DRIVERS="$DRIVERS drv_BWCT.o"
      AC_DEFINE(WITH_BWCT,1,[BWCT driver])
      DRVLIBS="$DRVLIBS -lusb"
   else
      AC_MSG_WARN(usb.h not found: BWCT driver disabled)
   fi
fi

if test "$CRYSTALFONTZ" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_Crystalfontz.o"
   AC_DEFINE(WITH_CRYSTALFONTZ,1,[Crystalfontz driver])
fi

if test "$CURSES" = "yes"; then
   if test "$has_curses" = true; then
      DRIVERS="$DRIVERS drv_Curses.o"
      DRVLIBS="$DRVLIBS $CURSES_LIBS"
      CPPFLAGS="$CPPFLAGS $CURSES_INCLUDES"
      AC_DEFINE(WITH_CURSES,1,[Curses driver])
   else
      AC_MSG_WARN(curses not found: Curses driver disabled)
   fi   
fi

if test "$CWLINUX" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_Cwlinux.o"
   AC_DEFINE(WITH_CWLINUX,1,[CwLinux driver])
fi

if test "$HD44780" = "yes"; then
   TEXT="yes"
   PARPORT="yes"
   DRIVERS="$DRIVERS drv_HD44780.o"
   AC_DEFINE(WITH_HD44780,1,[HD44780 driver])
fi

if test "$LCDLINUX" = "yes"; then
   TEXT="yes"
   DRIVERS="$DRIVERS drv_LCDLinux.o"
   AC_DEFINE(WITH_LCDLINUX,1,[LCD-Linux driver])
fi

if test "$LCDTERM" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_LCDTerm.o"
   AC_DEFINE(WITH_LCDTERM,1,[LCDTerm driver])
fi

if test "$M50530" = "yes"; then
   TEXT="yes"
   PARPORT="yes"
   DRIVERS="$DRIVERS drv_M50530.o"
   AC_DEFINE(WITH_M50530,1,[M50530 driver])
fi

if test "$MATRIXORBITAL" = "yes"; then
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_MatrixOrbital.o"
   AC_DEFINE(WITH_MATRIXORBITAL,1,[MatrixOrbital driver])
fi

if test "$MILINST" = "yes"; then
   DRIVERS="$DRIVERS drv_MilfordInstruments.o"
   AC_DEFINE(WITH_MILINST,1,[Milford Instruments driver])
fi

if test "$NULL" = "yes"; then
   DRIVERS="$DRIVERS drv_NULL.o"
   AC_DEFINE(WITH_NULL,1,[NULL driver])
fi

if test "$PNG" = "yes"; then
   if test "$has_gd" = "true"; then
      GRAPHIC="yes"
      IMAGE="yes"
      AC_DEFINE(WITH_PNG,1,[ driver])
      DRVLIBS="$DRVLIBS -lgd"
   else
      AC_MSG_WARN(gd.h not found: PNG driver disabled)
   fi
fi

if test "$PPM" = "yes"; then
   GRAPHIC="yes"
   IMAGE="yes"
   AC_DEFINE(WITH_PPM,1,[ driver])
fi

if test "$IMAGE" = "yes"; then
   DRIVERS="$DRIVERS drv_Image.o"
fi

if test "$ROUTERBOARD" = "yes"; then
   TEXT="yes"
   DRIVERS="$DRIVERS drv_RouterBoard.o"
   AC_DEFINE(WITH_ROUTERBOARD,1,[RouterBoard driver])
fi

if test "$SIMPLELCD" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_SimpleLCD.o"
   AC_DEFINE(WITH_SIMPLELCD,1,[SimpleLCD driver])
fi

if test "$T6963" = "yes"; then
   GRAPHIC="yes"
   PARPORT="yes"
   DRIVERS="$DRIVERS drv_T6963.o"
   AC_DEFINE(WITH_T6963,1,[T6963 driver])
fi

if test "$USBLCD" = "yes"; then
   TEXT="yes"
   SERIAL="yes"
   DRIVERS="$DRIVERS drv_USBLCD.o"
   AC_DEFINE(WITH_USBLCD,1,[USBLCD driver])
   if test "$has_usb" = "true"; then
      DRVLIBS="$DRVLIBS -lusb"
   fi
fi

if test "$X11" = "yes"; then
   if test "$no_x" = "yes"; then
      AC_MSG_WARN(X11 headers or libraries not available: X11 driver disabled)
   else
      GRAPHIC="yes"
      DRIVERS="$DRIVERS drv_X11.o"
      DRVLIBS="$DRVLIBS -L$ac_x_libraries -lX11"
      AC_DEFINE(WITH_X11,1,[X11 driver])
   fi
fi

if test "$DRIVERS" = ""; then
   AC_MSG_ERROR([You should include at least one driver...])
fi
   
# generic text driver
if test "$TEXT" = "yes"; then
   DRIVERS="$DRIVERS drv_generic_text.o"
fi

# generic graphic driver
if test "$GRAPHIC" = "yes"; then
:
   DRIVERS="$DRIVERS drv_generic_graphic.o"
fi

# generic parport driver
if test "$PARPORT" = "yes"; then
   DRIVERS="$DRIVERS drv_generic_parport.o"
fi

# generic serial driver
if test "$SERIAL" = "yes"; then
   DRIVERS="$DRIVERS drv_generic_serial.o"
fi

AC_SUBST(DRIVERS)
AC_SUBST(DRVLIBS)
