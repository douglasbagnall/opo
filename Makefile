all::	opo

GDB_ALWAYS_FLAGS = -g
WARNINGS = -Wall -Wextra -Wno-unused-parameter

ARCH = $(shell arch)
ifeq "$(ARCH)" "x86_64"
ARCH_CFLAGS = -fPIC -DPIC -m64
else
ARCH_CFLAGS = -m32 -msse2
endif

ALL_CFLAGS = -march=native -pthread $(VECTOR_FLAGS) -O3 $(WARNINGS) -pipe  -D_GNU_SOURCE -std=gnu99 $(INCLUDES) $(ARCH_CFLAGS) $(CFLAGS) $(GDB_ALWAYS_FLAGS)
ALL_LDFLAGS = $(LDFLAGS)

VECTOR_FLAGS = -msse2 -DHAVE_SSE2 -D__SSE2__ -floop-strip-mine -floop-block

GST_INCLUDES =  -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2
INCLUDES = -I. $(GST_INCLUDES)

LINKS = -L/usr/local/lib -lgstbase-0.10 -lgstreamer-0.10 -lgobject-2.0 \
	-lglib-2.0 -lgstvideo-0.10 

SOURCES = 
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

clean:
	rm -f *.so *.o *.a *.d *.s

.c.o:
	$(CC)  -c -MD $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.s:	%.c
	$(CC)  -S  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

%.i:	%.c
	$(CC)  -E  $(ALL_CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: TAGS all rsync clean debug

GTK_APP = opo.c
GTK_LINKS = -lglib-2.0 $(LINKS) -lgstinterfaces-0.10 -lgtk-x11-2.0
GTK_INCLUDES = -I/usr/include/gtk-2.0/ -I/usr/include/cairo/ -I/usr/include/pango-1.0/ -I/usr/lib/gtk-2.0/include/ -I/usr/include/atk-1.0/ -I/usr/include/gdk-pixbuf-2.0/

opo::
	$(CC)  -g $(ALL_CFLAGS) $(CPPFLAGS) $(CV_LINKS) $(INCLUDES) $(GTK_INCLUDES)\
	  $(GTK_LINKS) -o $@ $(GTK_APP)

debug:
	make -B CFLAGS='-g -fno-inline -fno-inline-functions -fno-omit-frame-pointer -O0' opo

