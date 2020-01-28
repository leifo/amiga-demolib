##
##	$Id: makefile,v 23.2 93/05/24 15:43:32 chris Exp $
##	$Revision: 23.2 $
##
##	$Filename: Examples/makefile $
##	$Author: chris $
##	$Release: 21.1 $
##	$Date: 93/05/24 15:43:32 $
##
##	Makefile to build iff.library examples
##
##	COPYRIGHT (C) 1987-1992 BY CHRISTIAN A. WEBER, BRUGGERWEG 2,
##	CH-8037 ZUERICH, SWITZERLAND.
##	THIS FILE MAY BE FREELY DISTRIBUTED. USE AT YOUR OWN RISK.
##

ALL:	EasyExample AnimExample GrabScreen


#############################################################################
##	Flags

CFLAGS	= -cfustq -fid -ms -v -rr -s -O
AFLAGS	= -l -iINCLUDE: 
LFLAGS	= ADDSYM


#############################################################################
##	Rules

.s.o:
	Genam $(AFLAGS) $*.S

.c.o:
	LC $(CFLAGS) $*.c


#############################################################################
##	EasyExample

EX_STARTUP	= LIB:c.o
EX_MODULES	= EasyExample.o
EX_LIBS		= LIB:lcr.lib

EasyExample:	$(EX_STARTUP) $(EX_MODULES) $(EX_LIBS)
		BLINK FROM $(EX_STARTUP) $(EX_MODULES) TO $* LIB $(EX_LIBS)


#############################################################################
##	AnimEXample

AX_STARTUP	= LIB:c.o
AX_MODULES	= AnimExample.o
AX_LIBS		= LIB:lcr.lib

AnimExample:	$(AX_STARTUP) $(AX_MODULES) $(AX_LIBS)
		BLINK FROM $(AX_STARTUP) $(AX_MODULES) TO $* LIB $(AX_LIBS)


#############################################################################
##	GrabScreen

GS_STARTUP	= LIB:c.o
GS_MODULES	= GrabScreen.o
GS_LIBS		= LIB:lcr.lib

GrabScreen:	$(GS_STARTUP) $(GS_MODULES) $(GS_LIBS)
		BLINK FROM $(GS_STARTUP) $(GS_MODULES) TO $* LIB $(GS_LIBS)

