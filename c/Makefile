
GIMPTOOL = gimptool-2.0

CC = gcc

LIBS += -lm $(shell pkg-config --libs gimpui-2.0)

CFLAGS += $(shell pkg-config --cflags gimpui-2.0)

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

