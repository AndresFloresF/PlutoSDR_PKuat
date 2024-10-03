CC = gcc
CFLAGS = -I/usr/include
LDFLAGS = -L/usr/lib
LDLIBS = -lslabhidtosmbus -lslabhiddevice
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET = BMP180_CP2112
all: $(TARGET)
$(TARGET):  $(OBJECTS)
	$(CC)	$(LDFLAGS)	-o	$@	$^	$(LDLIBS)
    
%.o:	%.c
	$(CC)	-c	$<	$(CFLAGS)

clean:
	rm	-f	$(OBJECTS)	$(TARGET)

