# ---------------------------------------------------------------------------
#
#   Filename: wxaui Makefile
#   Project:  Wx Advanced User Interface (wxaui)
#   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
#
#   Developer(s):	Benjamin I. Williams
#
#   Changes:
#
#        3 Oct 2005 - Created
#
# ---------------------------------------------------------------------------


# change these two to match your setup
WX_DIR := ../supp/wxGTK-2.6.3
WX_CONFIG := ${WX_DIR}/lib/wx/config/inplace-gtk2-unicode-debug-static-2.6

WX_CFLAGS := $(shell ${WX_CONFIG} --prefix=${WX_DIR} --cppflags)
WX_LIBS := $(shell ${WX_CONFIG} --prefix=${WX_DIR} --libs)

DEFINES = 
CPP = g++
CFLAGS = ${WX_CFLAGS} -g -ggdb
INCLUDES = -I../

CFLAGS += -DwxUSE_ABOUTDLG=1

ABOUTDLG_OBJECTS = \
	aboutdlgg.o

all: aboutdlgg.a 

aboutdlgg.a: $(ABOUTDLG_OBJECTS)
	ar rc aboutdlgg.a $(ABOUTDLG_OBJECTS)
	ranlib aboutdlgg.a

clean:
	rm -f *.o aboutdlgg.a 

$(ABOUTDLG_OBJECTS): %.o : %.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<


