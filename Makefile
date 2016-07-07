VERSION = 0.0.1

VPATH := src
OBJS := main.o config.o video.o chars.o memory.o rom.o cpu.o debugger.o

#
# compilation options
#
CPPFLAGS = -std=c11 -DVERSION=\"$(VERSION)\" -D_GNU_SOURCE
ifdef FORCE_COLOR
  CPPFLAGS += -fdiagnostics-color=always
else
  CPPFLAGS += -fdiagnostics-color
endif

#
# add cflags/libraries
#
CPPFLAGS += `pkg-config --cflags sdl2`
LDFLAGS  += -fuse-ld=gold `pkg-config --libs sdl2`

#
# add warnings
#
ifndef W
  WARNINGS := @build/warnings.txt
  ifeq ($(CC),gcc)
    WARNINGS += @build/warnings_gcc.txt
  endif
endif

# 
# debug/release
# 
all:
	@echo Choose a target: 'debug' or 'release'.

debug: TARGET_CFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions
debug: TARGET_LDFLAGS = -g
debug: lilliput

release: TARGET_CFLAGS = -DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
release: TARGET_LDFLAGS = -flto -Wl,--strip-all
release: lilliput

profile: TARGET_CFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions -pg
profile: TARGET_LDFLAGS = -g -pg
profile: lilliput

#
# pull dependence info from existing .o files
#
-include $(OBJS:.o=.d)

# 
# compile source files
#
%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(TARGET_CFLAGS) $<
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# 
# other dependencies
#
data/font.bmp: data/font.png
	convert $< -depth 1 -monochrome $@

src/font.h: data/font.bmp
	xxd -i $< > $@

src/chars.c: src/font.h

#
# link
#
lilliput: $(OBJS)
	$(CC) $^ -o $@ $(TARGET_LDFLAGS) $(LDFLAGS)

# 
# install
#
install: lilliput
	cp lilliput /usr/local/bin/

uninstall:
	rm /usr/local/bin/lilliput

#
# other rules
#
test: debug
	ruby test/test.rb

cloc:
	cloc Makefile src/*.h src/*.c

check-leaks: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/lilliput.supp ./lilliput -D

gen-suppressions: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/lilliput.supp ./lilliput -D
	sed -i -e '/^==.*$$/d' build/lilliput.supp

clean:
	rm -f lilliput *.o *.d

.PHONY: debug release profile cloc check-leaks gen-suppressions clean install
