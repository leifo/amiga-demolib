##
##	$Id: makefile,v 1.0 92/11/19 21:49:35 chris Exp $
##	$Revision: 1.0 $
##
##	$Filename: ShowIFF/makefile $
##	$Author: chris $
##	$Release: $
##	$Date: 92/11/19 21:49:35 $
##
##	Makefile for ShowIFF (SAS/C 6.x)
##

PROJECT	= ShowIFF


#############################################################################
##	Flags

#CFLAGS	= DBG
AFLAGS	= -iINCLUDE: -l
LFLAGS	= SMALLDATA NODEBUG NOICONS MAP RAM:slink.map h,s,f,o,plain


#############################################################################
##	Rules

.c.o:
		sc $(CFLAGS) $*.c

.S.o:
		Genam $(AFLAGS) $*.S


#############################################################################
##	Command line options

All:		$(PROJECT)

Bump:
		@Bumper $(PROJECT)
		@SMake


#############################################################################
##	$(PROJECT)

SI_STARTUP	= CCLIB:ArgsStartup20.o
SI_LIBS		= LIB:amiga.lib CCLIB:ch.lib
SI_INCL		= 
SI_MODS		= $(PROJECT).o

$(SI_MODS):	$(SI_INCL)

$(PROJECT):	$(SI_STARTUP) $(SI_MODS) $(SI_LIBS)
		@SLINK $(LFLAGS) FROM $(SI_STARTUP) $(SI_MODS) TO $(PROJECT) LIB $(SI_LIBS)

$(PROJECT).o:	$(PROJECT)_rev.h

