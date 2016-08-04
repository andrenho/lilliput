VERSION = 0.2

VPATH := src lib

OBJS_LIB := luisavm.o cpu.o video.o test.o assembler.o \
	debugger.o debuggerhelp.o debuggermemory.o debuggernotimplemented.o

.DEFAULT_GOAL := release

#
# compilation options
#
CPPFLAGS = -std=c++14 -DVERSION=\"$(VERSION)\" -D_GNU_SOURCE
ifdef FORCE_COLOR
  CPPFLAGS += -fdiagnostics-color=always
else
  CPPFLAGS += -fdiagnostics-color
endif

#
# add cflags/libraries
#
CPPFLAGS += -fpic -Ilib

ifeq ($(OS),Windows_NT)
  SOFLAGS += -Wl,--out-implib,libluisavm.a
else
  LDFLAGS += -fuse-ld=gold
endif

#
# add warnings
#
ifndef W
  WARNINGS := @build/warnings.txt
  ifeq ($(CXX),g++)
    WARNINGS += @build/warnings_gcc.txt
  endif
endif

# 
# debug/release
# 
all:
	@echo Choose a target: 'debug' or 'release'.

debug: TARGET_CPPFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions
debug: TARGET_LDFLAGS = -g
debug: luisavm las libluisavm.so

release: TARGET_CPPFLAGS = -DNDEBUG -Ofast -fomit-frame-pointer -ffast-math -mfpmath=sse -fPIC -msse -msse2 -msse3 -mssse3 -msse4 -flto
release: TARGET_LDFLAGS = -flto -Wl,--strip-all
release: luisavm las libluisavm.so

profile: TARGET_CPPFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions -pg
profile: TARGET_LDFLAGS = -g -pg
profile: luisavm

#
# pull dependence info from existing .o files
#
-include $(OBJS:.o=.d)

# 
# compile source files
#
%.o: %.cc
	$(CXX) -c $(WARNINGS) $(CPPFLAGS) $(TARGET_CPPFLAGS) $<
	@$(CXX) -MM $(CPPFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

main.o: CPPFLAGS += -fpic -Ilib `pkg-config --cflags sdl2`

# 
# other dependencies
#
lib/font.xbm: data/font.png
	convert $< -depth 1 -monochrome $@
	sed -i 's/static char/static unsigned char/g' $@

lib/video.cc: lib/font.xbm

#
# link
#
luisavm: libluisavm.so main.o
	$(CXX) main.o -o $@ $(TARGET_LDFLAGS) $(LDFLAGS) -Wl,-rpath=. -L. -lluisavm `pkg-config --libs sdl2`

las: libluisavm.so las.o
	$(CXX) las.o -o $@ $(TARGET_LDFLAGS) $(LDFLAGS) -Wl,-rpath=. -L. -lluisavm

libluisavm.so: $(OBJS_LIB)
	$(CXX) -shared $^ -o $@ $(TARGET_LDFLAGS) $(LDFLAGS) $(SOFLAGS)

luisavm-tests: libluisavm.so tests.o
	$(CXX) tests.o -o $@ $(TARGET_LDFLAGS) $(LDFLAGS) -Wl,-rpath=. -L. -lluisavm

# 
# install
#
dist:
	mkdir luisavm-$(VERSION)
	cp --parents doc/* build/*.txt data/* lib/*.cc lib/*.hh \
		lib/*.xbm LICENSE Makefile src/*.cc luisavm-$(VERSION)
	tar -czf luisavm-$(VERSION).tar.gz luisavm-$(VERSION)
	rm -rf luisavm-$(VERSION)/

distcheck: dist
	tar zxpvf luisavm-$(VERSION).tar.gz
	rm luisavm-$(VERSION).tar.gz
	cd luisavm-$(VERSION) && make test
	rm -rf luisavm-$(VERSION)

install: luisavm las
	cp libluisavm.so /usr/lib
	cp luisavm /usr/local/bin/
	cp las /usr/local/bin/
	cp lib/luisavm.h /usr/local/include
	ldconfig

uninstall:
	rm /usr/lib/libluisavm.so
	rm /usr/local/bin/luisavm
	rm /usr/local/bin/las
	rm /usr/local/include/luisavm.h

#
# other rules
#
test: TARGET_CPPFLAGS = -g -ggdb3 -O0 -DDEBUG -fno-inline-functions
test: TARGET_LDFLAGS = -g
test: luisavm-tests
	./luisavm-tests

cloc:
	cloc Makefile src/*.hh src/*.cc lib/*.hh lib/*.cc

check-leaks: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=build/luisavm.supp ./luisavm-tests

gen-suppressions: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-limit=no --gen-suppressions=all --log-file=build/luisavm.supp ./luisavm-tests
	sed -i -e '/^==.*$$/d' build/luisavm.supp

lint:
	clang-tidy lib/*.hh lib/*.cc src/*.cc "-checks=*,-google-build-using-namespace,-google-readability-todo,-cppcoreguidelines-pro-type-reinterpret-cast,-cppcoreguidelines-pro-bounds-array-to-pointer-decay,-cppcoreguidelines-pro-type-const-cast,-cert-err52-cpp,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-pro-type-union-access,-cppcoreguidelines-pro-bounds-constant-array-index,-clang-analyzer-alpha.core.CastToStruct" -- -I. -Ilib --std=c++14 -DVERSION=\"$(VERSION)\"

clean:
	rm -f luisavm libluisavm.so *.o *.d

.PHONY: debug release profile cloc check-leaks gen-suppressions clean install test
