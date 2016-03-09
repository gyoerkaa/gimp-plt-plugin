
GIMPTOOL = gimptool-2.0
#	$(GIMPTOOL) --build src/file-bioplt.c

CC = gcc

LIBS := $(shell $(GIMPTOOL) --libs) -lm

CFLAGS := $(shell $(GIMPTOOL) --cflags)

OUTDIR = bin

TARGET = $(OUTDIR)/file-bioplt

SOURCES +=  src/file-bioplt.c

HEADERS	+=  src/file-bioplt.h

all:
	mkdir -p $(OUTDIR)
	$(CC) $(SOURCES) $(HEADERS) $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

install:
	$(GIMPTOOL) --install-bin $(TARGET)

