###########################################
# Makefile for cp2615smt
# Requires libusb-1.0
# Requires libuuid
# Requires libslabhidtosmbus, libslabhiddevice
#
# Silicon Labs
# Modified March 30th, 2017
###########################################

CPU_ARCH?=$(shell arch)
BUILD ?=../build
BUILDPATH ?=$(BUILD)/bin/$(CPU_ARCH)
OBJPATH ?=$(BUILD)/obj/$(CPU_ARCH)

# required packages
PKG_CONFIG_DEPENDENCIES = libusb-1.0

CSRCS =
CPPSRCS  = cp2615smt.cpp
CPPSRCS += cp2615.cpp 
CPPSRCS += util.cpp
CPPSRCS += OsDep.cpp
CPPSRCS += smt.cpp

COBJS=$(CSRCS:%.c=%.o)
CPPOBJS=$(CPPSRCS:%.cpp=%.o)

OBJFILES = $(COBJS) $(CPPOBJS)

OBJFULLPATHS=$(addprefix $(OBJPATH)/, $(OBJFILES))

EXENAME = cp2615smt
EXEFULLPATH = $(BUILDPATH)/$(EXENAME)

LIBS	  += -lslabhidtosmbus
LIBS	  += -lslabhiddevice
LIBS	  += -luuid
LIBS      += -lpthread
LIBS      += $(foreach deplib,$(PKG_CONFIG_DEPENDENCIES), `pkg-config --libs $(deplib)`)

LDFLAGS += "-Wl,-rpath=/usr/local/lib/"
CC       ?= gcc
CFLAGS   ?= -Wall -fPIC -g $(INCLUDES)

CXX      ?= g++
CXXFLAGS ?= -Wall -fPIC -g $(INCLUDES)

all: $(OBJPATH) $(BUILDPATH) $(EXEFULLPATH)

$(OBJPATH):
	mkdir -p $@

$(BUILDPATH):
	mkdir -p $@

$(OBJPATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

$(EXEFULLPATH): $(OBJFULLPATHS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) $(LIBPATHS) -o $@
	

clean:
	rm -rf $(BUILD)


