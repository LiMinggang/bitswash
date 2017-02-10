
# ---------------------------------------------------------------------------
#
#   Filename: wxauitest Makefile
#   Project:  Dark Cloud
#   (C) Copyright 2002-2004, Kirix Corporation, All Rights Reserved.
#
#   Developer(s):	Benjamin I. Williams
#
#   Changes:
#
#       16 Dec 2002 - Created
#       31 Mar 2004 - Updated for KPP 2.x
#
# ---------------------------------------------------------------------------


WX_DIR := ../../supp/wxGTK-2.6.2
WX_CONFIG := ${WX_DIR}/lib/wx/config/inplace-gtk2-unicode-debug-static-2.6

WX_CFLAGS := $(shell ${WX_CONFIG} --prefix=${WX_DIR} --cppflags)
WX_LIBS := $(shell ${WX_CONFIG} --prefix=${WX_DIR} --libs)



EXENAME = wxauitest
INCLUDES = -I../include
CPP = g++
CFLAGS = $(WX_CFLAGS)
LIBS = ../lib/libwxaui.a $(WX_LIBS)

OBJECTS = wxauitest.o


all: $(EXENAME)
    
$(EXENAME): $(OBJECTS) 
		$(CPP) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(EXENAME)

clean:
	rm -f *.o *.gch $(EXENAME)

$(OBJECTS): %.o : %.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c $<

