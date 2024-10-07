###############################################################################
#
# Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
###############################################################################
 
SHELL = /bin/sh

ifeq ($(NAS_ARCH), ARM)
CC = aarch64-linux-gnu-g++
else
CC = g++
endif

CFLAGS = -O -Wall -g -DPRODUCT_TSERIES -fPIC
LIBDIR = -L../lib
LIBDIR += -L../../base/cmr_log
LIBDIR += -L../../base/cmr_safefun

USERSPACEDIR= ../..
INCLUDES = -I../lib
INCLUDES += -I$(USERSPACEDIR)
INCLUDES += -I$(USERSPACEDIR)/base
INCLUDES += -I/usr/local/include -I/usr/local/ssl/include/
INCLUDES += -I/home/shuai/Module/platform/SecureCLib_rel/include
INCLUDES += -I/usr/include/tirpc/
ifeq ($(ARM_VERSION), ARM1620)
INCLUDES += -I/usr/include/tirpc/
endif

ifeq ($(ARM_VERSION), ARM1620)
CFLAGS+=-D_ARM1620_ 
endif

DEFINES =  -DDEBUG 
 
LD = $(CC)
LDFLAGS = 
# These should work for solaris 2.5
#LIBS = -L$(LIBDIR) -lndmp -lsocket -lnsl
# These should work for linux
LIBS = -L../lib/ -lndmp  
# LIBS+= -L../../cmr_kagent_api/ -lkagentapi
# LIBS+= -L../../cmr_cm_api/ -lcmrcmapi
# LIBS+= -L../../cmr_store/api/ -lcmrstoreapi
# LIBS+= -L../../base/cmr_log/ -lcmrlog 
# LIBS+= -L../../base/cmr_safefun/ -lcmrsafefun 
# LIBS+= -L../../base/cmr_msg/ -lcmrmsg 
# LIBS+= -L../../base/cmr_common/ -lcmrcommon
LIBS+= -L/usr/local/ssl/lib/ -lcrypto -lssl
# LIBS+= -L/usr/local/lib -lboost_thread -lboost_system -lpthread -lprotobuf -lzmq -ldl
LIBS+= -L/usr/local/lib -lpthread -ldl
LIBS+= -L/home/shuai/Module/third_open_src/protobuf_rel/lib -lprotobuf
LIBS+= -L/home/shuai/Module/third_open_src/boost_rel/lib/ -lboost_thread -lboost_system
LIBS+= -L/home/shuai/Module/platform/SecureCLib_rel/lib -lsecurec
# ifeq ($(ARM_VERSION), ARM1620)
LIBS+= -ltirpc
# endif
LDFLAGS+=-Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -pie
 
TARGET = ndmpcopy
SOURCES = main.c dump.c queue.c file_history.c handler_tbl.c log.c notify.c
OBJS    = $(SOURCES:.c=.o)
OBJECTS = 
 
 
all default:	$(OBJS) $(OBJECTS) 
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
 
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<
 
clean:
	rm -f *.o *~ core $(TARGET)
 

