TOPDIR = .
CFLAGS  = -I$(TOPDIR) -I$(TOPDIR)/expat
CFLAGS += -MD -MP
CFLAGS += -Wall
CFLAGS += -O0 -g3
CFLAGS += -DHAVE_EXPAT_CONFIG_H

SOURCES=$(wildcard $(TOPDIR)/*.c $(TOPDIR)/expat/*.c)
OBJECTS=$(SOURCES:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.SUFFIXES:

all: fixperms

fixperms: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *.d expat/*.o expat/*.d fixperms

-include $(SOURCES:.c=.d)

.PHONY: all clean
