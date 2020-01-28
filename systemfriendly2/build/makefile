# 30.04.18, Leif Oppermann, lop@gmx.de
# updated 28.01.20 for vbcc 0.9f toolchain and NDK location based on Leffmann's vbcc-windows

TARGET = main

SRCDIR= $(realpath ../src)
SHAREDDIR= $(realpath ../../shared)

SOURCES = \
   $(SRCDIR)/main.c \
   $(SRCDIR)/demolib.s

	
#vpath %.c src
#vpath %.asm src

CFLAGS = -O=23999 -cpu=68000 -fpu=none -c99 -DAMIGA -speed -k
ASFLAGS =
LDFLAGS =

INCDIR = $(SHAREDDIR)

VBCCDIR= $(realpath $(VBCC))
#LIBDIR =

# "amiga vc" is the minimum
# m040 m881 posix
LIBS = amiga vc 

#CC       = vbccm68k
CC       = vc
AS       = vasmm68k_mot
LD       = vlink

# Add includes and libraries.
INCDIR   := $(VBCCDIR)/NDK39/Include/include_h $(VBCCDIR)/targets/m68k-amigaos-posix/include $(SRCDIR)/misc $(INCDIR)
LIBDIR   := $(VBCCDIR)/targets/m68k-amigaos/lib $(VBCCDIR)/targets/m68k-amigaos-posix/lib

# C above, assembler below
ASINCDIR   := $(VBCCDIR)/NDK39/Include/include_i $(SRCDIR)/misc

# Strip everything if there's no debug infos
ifeq ($(findstring -g, $(CFLAGS)), -g)
else
 LDFLAGS  := $(LDFLAGS) -s
endif

# Set the math library to use
ifeq ($(findstring -fpu=68881, $(CFLAGS)), -fpu=68881)
 LDFLAGS  := $(LDFLAGS) -lm881
else
 ifeq ($(findstring -fpu=68040, $(CFLAGS)), -fpu=68040)
  LDFLAGS  := $(LDFLAGS) -lm040
 else
  ifeq ($(findstring -fpu=68060, $(CFLAGS)), -fpu=68060)
   LDFLAGS  := $(LDFLAGS) -lm881
  else
   LDFLAGS  := $(LDFLAGS) -lmieee
  endif
 endif
endif

# cflags:
# - temporarily removed: -quiet		
# - add -k if you want to see the *.asm temporary output
# - 01.03.2015 - removed -c (reported as " Unknown Flag <-c>", maybe due to newer tools?)
# profiling with -prof (https://github.com/kusma/vbcc/blob/master/doc/vprof.texi)
CFLAGS   := -c -maxoptpasses=100 -inline-depth=100 -inline-size=30 -hunkdebug $(addprefix -I, $(INCDIR)) -D__VBCC__ $(CFLAGS) -notmpfile

ASFLAGS  := -quiet -Fhunk -devpac $(ASFLAGS) $(addprefix -I, $(ASINCDIR))

#            -lamiga \
#            -lvc \

LDFLAGS  := $(addprefix -L,$(LIBDIR)) \
            "$(VBCC)/targets/m68k-amigaos/lib/startup.o" \
            $(addprefix -l,$(LIBS)) \
            -Bstatic -bamigahunk -Cvbcc $(LDFLAGS)


#
# lists
#

LINKOBJ   = $(patsubst %.c, %.o, $(SOURCES))
LINKOBJ   := $(patsubst %.s, %.o, $(LINKOBJ))

DELOBJ1   = $(patsubst %.c, %.asm, $(SOURCES))
DELOBJ1   := $(patsubst %.s, %.o, $(DELOBJ1))
DELOBJ2   = $(patsubst %.c, %.o, $(SOURCES))
DELOBJ2   := $(patsubst %.s, , $(DELOBJ2))

OBJC     = $(patsubst %.c, %.o, $(SOURCES))
OBJASM   = $(patsubst %.s, %.o, $(SOURCES))

all: $(TARGET)

#
# link	
#

$(TARGET): $(OBJC) $(OBJASM)
	@echo "******* Linking: $(TARGET)"
#	@echo $(LIBDIR)
#	@echo " "
	@echo $(LDFLAGS)
	@echo " " 
	$(LD) $(LDFLAGS) -o $(TARGET) $(LINKOBJ) 
	@echo " " 

#
# compile 
#

%.o : $(SRCDIR)/%.c
	@echo "******* Compiling $< to $@"
#	$(CC) $(CFLAGS) -o $(notdir $@) $<
	$(CC) $(CFLAGS) -o $@ $<
#	$(CC) $(CFLAGS) -o $@ $(SRCDIR)/$<
	@echo " "

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

#
# clean
#

clean:
	-rm -f $(TARGET)
	-rm -f $(DELOBJ1)
	-rm -f $(DELOBJ2)
            

            

