###########################################
# Makefile for libcp210xruntime
#
# Silicon Labs
# Modified March 6, 2019
###########################################

CPU_ARCH?=$(shell arch)
BUILD?=./build
BUILDPATH?=$(BUILD)/lib/$(CPU_ARCH)
OBJPATH?=$(BUILD)/obj/$(CPU_ARCH)
APPPATH?=$(BUILD)/bin/$(CPU_ARCH)

COMMON?=../../../host_common

LIB_BASE_NAME = cp210xruntime
LIB_NAME = lib$(LIB_BASE_NAME)
LIB_VER = 1.0
LIB_SOVER = 1

SONAME=$(LIB_NAME).so.$(LIB_SOVER)
LIBFULLNAME=$(LIB_NAME).so.$(LIB_VER)

LIBFULLPATH=$(BUILDPATH)/$(LIBFULLNAME)
ARCHIVE_FULL_PATH=$(BUILDPATH)/$(LIB_NAME).a

CSRCS =
CPPSRCS = CP210xRuntime.cpp

COBJS=$(CSRCS:%.c=%.o)
CPPOBJS=$(CPPSRCS:%.cpp=%.o)

OBJFILES = $(COBJS) $(CPPOBJS)

OBJFULLPATHS=$(addprefix $(OBJPATH)/, $(OBJFILES))

INCLUDES ?= -I$(COMMON)/include/

CC       ?= gcc
CFLAGS   ?= -Wall -fPIC -g $(INCLUDES)

CXX      ?= g++
CXXFLAGS ?= -Wall -fPIC -g $(INCLUDES)

AR       ?= ar
ARFLAGS  ?= rcs

all: 
	echo "lib - build shared library"
	echo "archive - build archive"
	echo "app - build test app"
	echo "debug - print some stuff"
	echo "clean - obvious"
	
	
lib: $(LIBFULLPATH)

archive: $(ARCHIVE_FULL_PATH)

$(OBJPATH)/%.o: %.c 
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(OBJPATH)/%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

$(OBJPATH):
	mkdir -p $@

$(BUILDPATH):
	mkdir -p $@
	
$(APPPATH):
	mkdir -p $@


$(LIBFULLPATH): $(OBJPATH) $(BUILDPATH) $(OBJFULLPATHS)
	$(CXX) -shared -Wl,-soname,$(SONAME) -o $@ $(OBJFULLPATHS) $(LIBS)
	cd $(BUILDPATH) && \
	ln -sf $(LIBFULLNAME) $(SONAME) && \
	ln -sf $(SONAME) $(LIB_NAME).so
	

$(ARCHIVE_FULL_PATH): $(OBJPATH) $(BUILDPATH) $(OBJFULLPATHS)
	$(AR) $(ARFLAGS) $(BUILDPATH)/$(LIB_NAME).a $(OBJFULLPATHS)

debug:
	echo "BUILDPATH: "$(BUILDPATH)
	echo "OBJPATH: "$(OBJPATH)
	echo "OBJFULLPATHS: "$(OBJFULLPATHS)

clean:
	rm -rf $(BUILDPATH)
	rm -rf $(OBJPATH)
	rm -rf $(BUILD)


app: $(APPPATH)
	$(CXX) $(ARCH) -g $(INCLUDES) -o $(APPPATH)/cp210xruntimeapp CP210xRuntimeApp.cpp  -L$(BUILDPATH) -l$(LIB_BASE_NAME) -lusb-1.0
