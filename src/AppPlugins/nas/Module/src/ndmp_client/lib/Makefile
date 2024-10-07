############################ -*- Mode: Makefile -*- ###########################
## Makefile
## 
## Description     : NDMP library Makefile
## 
## Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
##
## Permission is hereby granted, free of charge, to any person obtaining
## a copy of this software and associated documentation files (the
## "Software"), to deal in the Software without restriction, including
## without limitation the rights to use, copy, modify, merge, publish,
## distribute, sublicense, and/or sell copies of the Software, and to
## permit persons to whom the Software is furnished to do so, subject to
## the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
## DEALINGS IN THE SOFTWARE.
##
###############################################################################

.KEEP_STATE:

ifeq ($(NAS_ARCH), ARM)
CC = aarch64-linux-gnu-g++
else
CC = g++
endif

USERSPACEDIR= ../..
INCLUDES = -I$(USERSPACEDIR)
INCLUDES += -I/usr/local/include

ifeq ($(ARM_VERSION), ARM1620)
INCLUDES += -I/usr/include/tirpc/
endif

INCLUDES += -I$(USERSPACEDIR)/base
INCLUDES += -I$(USERSPACEDIR)/../../../include -I/usr/local/ssl/include/
INCLUDES += -I/usr/include/tirpc/

CFLAGS = -g -Wall -W -Wstrict-prototypes $(INCLUDES) -DPRODUCT_TSERIES -fPIC

ifeq ($(ARM_VERSION), ARM1620)
CFLAGS+=-D_ARM1620_ 
endif

# ifeq ($(ARM_VERSION), ARM1620)
LIBS = -ltirpc
# endif

SRCS = comm.c ndmp_xdr.c
OBJS = $(SRCS:.c=.o)

AR = ar

all: libndmp.a ndmp.h

libndmp.a: $(OBJS)
	$(AR) rv $@ $?

ndmp.h: ndmp.x
	rpcgen -C -h -o $@ ndmp.x

ndmp_xdr.c: ndmp.x
	rpcgen -c -o $@ ndmp.x

ndmp_xdr.o: ndmp_xdr.c ndmp.h
	$(CC) -c $(CFLAGS) -g ndmp_xdr.c

${OBJS}: ndmp.h

tags:
	etags -t *.[ch]

clean:
	rm -f libndmp.a *.o core *~ TAGS .make.state ndmp.h ndmp_xdr.c
