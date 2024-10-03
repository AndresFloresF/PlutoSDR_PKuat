###########################################
# Simple Makefile for SLABHIDtoUART HidUart
# example application
#
# Christopher Menke
# Silicon Labs
# 09-17-2012
###########################################

all: hidUartExample

OUT      ?= build/

CC       ?= gcc
CFLAGS   ?= -Wall -g -Wl,-rpath=/usr/local/lib

CXX      ?= g++
CXXFLAGS ?= -Wall -g -Wl,-rpath=/usr/local/lib

COBJS     = 
CPPOBJS   = main.o Terminal.o
OBJS      = $(COBJS) $(CPPOBJS)
LDFLAGS  ?= -L/usr/local/lib
LIBS     ?= -lslabhidtouart -lslabhiddevice -lpthread -lusb-1.0
INCLUDES ?= 


hidUartExample: $(OBJS) 
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $(OUT)hidUartExample
	
$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS) $(OUT)hidUartExample
	rm -rf $(OUT)

.PHONY: clean

