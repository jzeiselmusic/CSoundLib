# Compiler and flags
CC = clang 
CFLAGS = -std=c17 -Wno-incompatible-pointer-types-discards-qualifiers -arch arm64 
INCLUDES = -I./inc 

DYNAMIC_CFLAGS = -fPIC
DYNAMIC_LDFLAGS = -framework CoreAudio -framework AudioToolbox -framework CoreFoundation -dynamiclib -install_name @rpath/libcsoundlib.dylib

BUILT_STATIC = false
BUILT_DYNAMIC = false

# Source files
SRCS = src/csl_util.c src/effects.c src/devices.c src/streams.c src/init.c src/state.c src/hash.c src/wav.c src/csl_types.c src/track.c src/pocketfft.c
# Object files
OBJS = out/csl_util.o out/devices.o out/effects.o out/streams.o out/init.o out/state.o out/hash.o out/wav.o out/csl_types.o out/track.o out/pocketfft.o

out/csl_util.o: src/csl_util.c inc/csl_util.h inc/csl_types.h inc/state.h 
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/devices.o: src/devices.c inc/devices.h inc/csl_types.h inc/csl_util.h inc/streams.h inc/init.h inc/state.h inc/errors.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/streams.o: src/streams.c inc/csl_types.h inc/streams.h inc/devices.h inc/csl_util.h inc/init.h inc/state.h inc/wav.h inc/errors.h inc/track.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/init.o: src/init.c inc/init.h inc/errors.h inc/csl_types.h inc/streams.h inc/devices.h inc/state.h inc/wav.h inc/csoundlib.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/state.o: src/state.c inc/state.h inc/csoundlib.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/hash.o: src/hash.c inc/hash.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/wav.o: src/wav.c inc/wav.h 
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/csl_types.o: src/csl_types.c inc/csl_types.h inc/state.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/track.o: src/track.c inc/track.h inc/state.h inc/errors.h inc/csl_util.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/effects.o: src/effects.c inc/csoundlib.h inc/track.h inc/state.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@
out/pocketfft.o: src/pocketfft.c inc/pocketfft.h
	$(CC) $(CFLAGS) $(DYNAMIC_CFLAGS) $(INCLUDES) -c $< -o $@

# Target library
STATIC_TARGET = libcsoundlib.a

DYNAMIC_TARGET = libcsoundlib.dylib

# Default rule to build everything
all: outdir $(STATIC_TARGET) $(DYNAMIC_TARGET)

static: outdir $(STATIC_TARGET)

dynamic: outdir $(DYNAMIC_TARGET)

outdir:
	mkdir -p out

# this rule means that if you want to build with libcsoundlib you need all the frameworks used 
# in libsoundio - coreaudio, audiotoolbox, etc. This is probably worse than in you just needed to 
# download libsoundio to /usr/local/lib in order to build 

# Rule to create the static library
$(STATIC_TARGET): $(OBJS)
	mkdir temp && cd temp && ar x /usr/local/lib/libsoundio.a
	cd ..
	ar rcs out/$@ $(OBJS) temp/*.o
	rm -rf temp
	@$(eval BUILT_STATIC=true)

$(DYNAMIC_TARGET): $(OBJS)
	$(CC) -arch arm64 $(DYNAMIC_LDFLAGS) -o out/$(DYNAMIC_TARGET) -L/usr/local/lib -lsoundio $(OBJS)
	rm out/*.o
	install_name_tool -id @rpath/libcsoundlib.dylib /usr/local/lib/libcsoundlib.dylib
	@$(eval BUILT_DYNAMIC=true)

# Clean rule to remove object files
clean:
	rm -f $(OBJS) out/*.a out/*.dylib
	rm -rf temp

install:
	@if [ $(BUILT_STATIC) = true ]; then \
		mv out/$(STATIC_TARGET) /usr/local/lib/; \
	fi
	@if [ $(BUILT_DYNAMIC) = true ]; then \
		mv out/$(DYNAMIC_TARGET) /usr/local/lib/; \
	fi
	cp inc/csoundlib.h /usr/local/include/csoundlib.h

.PHONY: all clean
