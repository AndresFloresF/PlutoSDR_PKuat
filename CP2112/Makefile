all: CP2112

OUT      ?= build/

CC       ?= gcc
CFLAGS   ?= -Wall -g -no-pie -Wl,-rpath=/usr/local/lib

CXX      ?= g++
CXXFLAGS ?= -Wall -g -no-pie -Wl,-rpath=/usr/local/lib

COBJS     = 
CPPOBJS   = BMP180_CP2112.o Utilities.o
OBJS      = $(COBJS) $(CPPOBJS)
LDFLAGS  ?= -L/usr/local/lib
LIBS     ?= -lslabhidtosmbus -lslabhiddevice -lpthread -lusb-1.0
INCLUDES ?=


CP2112: $(OBJS) 
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $(OUT)CP2112
	
$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS) $(OUT)CP2112
	rm -rf $(OUT)

.PHONY: clean

