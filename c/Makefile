
GIMPTOOL = gimptool-2.0

CC = gcc

ifeq ($(OS), Windows_NT)
    $(info OS is Windows)
    EXT = .exe
    LDFLAGS = -mwindows
    PATHSEP = \\
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Linux)
        $(info OS is Linux)
    endif
    ifeq ($(UNAME_S), Darwin)
        $(info OS is OSX)
    endif
    EXT =
    LDFLAGS =
    PATHSEP = /
endif

OUTDIR  = bin

OUTFILE = file-bioplt

TARGET = $(OUTDIR)/$(OUTFILE)$(EXT)

LIBS += -lm $(shell pkg-config --libs gtk+-2.0 gimpui-2.0)

CFLAGS += $(shell pkg-config --cflags gtk+-2.0 gimpui-2.0)

SOURCES += src/file-bioplt.c

HEADERS	+= src/file-bioplt.h

all:
	mkdir -p $(OUTDIR)
	$(CC) $(SOURCES) $(HEADERS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

install:
	$(GIMPTOOL) --install-bin $(OUTDIR)$(PATHSEP)$(OUTFILE)$(EXT)
