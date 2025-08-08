
OSTYPE := $(shell uname -msr)

CFLAGS	:= -Wall -O2 -DNDEBUG
STRIP	:= strip
LDFLAGS	:= -mwindows -O2

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC	:= i686-pc-mingw32-gcc
RC	:= i686-pc-mingw32-windres 
CC	:= gcc
RC	:= windres 
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC	:= gcc
RC	:= windres 
endif

ifeq ($(findstring MINGW64,$(OSTYPE)),MINGW64)
CC	:= gcc
RC	:= windres 
endif


ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC	:= i586-mingw32msvc-gcc
RC	:= i586-mingw32msvc-windres
endif

SRC := src/focus_fader.c
OBJ := bin/focus_fader.o

RC	:= src/focus_fader.rc 
RES := bin/focus_fader.res

TARGET	:= bin/focus_fader.exe
OBJS	:= $(OBJ) $(RES)

all: $(TARGET)

.SUFFIXES: .rc .res .coff .c .o .od

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(RES): $(RC)
	windres $< -O coff -o $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	$(STRIP) $(TARGET)

clean: 
	rm -f $(OBJS)
