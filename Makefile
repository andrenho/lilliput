VERSION = 0.0.1

VPATH := src lib

OBJS_EXE := main.o
OBJS_LIB := log.o cpu.o computer.o device.o rom.o video.o palette.o

#
# compilation options
#
CFLAGS = -std=c11 -DVERSION=\"$(VERSION)\" -D_GNU_SOURCE
ifdef FORCE_COLOR
  CFLAGS += -fdiagnostics-color=always
else
  CFLAGS += -fdiagnostics-color
endif

#
# add cflags/libraries
#
CFLAGS += -fpic -Ilib `pkg-config --cflags sdl2`
LDFLAGS  += -fuse-ld=gold

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
debug: luisavm libluisavm.so

release: TARGET_CFLAGS = -DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
release: TARGET_LDFLAGS = -flto -Wl,--strip-all
release: luisavm libluisavm.so

profile: TARGET_CFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions -pg
profile: TARGET_LDFLAGS = -g -pg
profile: luisavm

#
# pull dependence info from existing .o files
#
-include $(OBJS:.o=.d)

# 
# compile source files
#
%.o: %.c
	$(CC) -c $(WARNINGS) $(CFLAGS) $(TARGET_CFLAGS) $<
	@$(CC) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# 
# other dependencies
#
lib/font.xbm: data/font.png
	convert $< -depth 1 -monochrome $@
	sed -i 's/static char/static unsigned char/g' $@

lib/video.c: lib/font.xbm

#
# link
#
luisavm: libluisavm.so $(OBJS_EXE)
	$(CC) $(OBJS_EXE) -o $@ $(TARGET_LDFLAGS) $(LDFLAGS) -Wl,-rpath=. -L. -lluisavm `pkg-config --libs sdl2`

libluisavm.so: $(OBJS_LIB)
	$(CC) -shared $^ -o $@ $(TARGET_LDFLAGS) $(LDFLAGS)

bindings/lua/luisavm.so: bindings/lua/luisavm.c libluisavm.so
	$(CC) bindings/lua/luisavm.c -shared -o $@ $(CFLAGS) $(TARGET_CFLAGS) `pkg-config --cflags --libs lua` -Wl,-rpath=. -L. -lluisavm

# 
# install
#
install: luisavm
	cp libluisavm.so /usr/lib
	cp luisavm /usr/local/bin/
	cp lib/luisavm.h /usr/local/include
	ldconfig

uninstall:
	rm /usr/lib/libluisavm.so
	rm /usr/local/bin/luisavm
	rm /usr/local/include/luisavm.h

#
# other rules
#
test: TARGET_CFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions
test: TARGET_LDFLAGS = -g
test: bindings/lua/luisavm.so
	@LUA_CPATH="$LUA_CPATH;bindings/lua/?.so" lua test/test.lua

debug-test: TARGET_CFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions
debug-test: TARGET_LDFLAGS = -g
debug-test: bindings/lua/luisavm.so
	@LUA_CPATH="$LUA_CPATH;bindings/lua/?.so" gdb --args lua test/test.lua

cloc:
	cloc Makefile src/*.h src/*.c lib/*.h lib/*.c test/*.lua bindings/lua/*.c

check-leaks: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/luisavm.supp ./luisavm -D

gen-suppressions: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/luisavm.supp ./luisavm -D
	sed -i -e '/^==.*$$/d' build/luisavm.supp

clean:
	rm -f luisavm libluisavm.so *.o *.d bindings/lua/luisavm.so

.PHONY: debug release profile cloc check-leaks gen-suppressions clean install test
