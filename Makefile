# Compiler and flags
CC = clang
CFLAGS = -std=c17 -Wno-incompatible-pointer-types-discards-qualifiers -arch arm64
INCLUDES = -I./inc 

# Source files
SRCS = src/csl_util.c src/devices.c src/streams.c src/init.c src/state.c src/hash.c src/wav.c src/csl_types.c src/track.c
# Object files
OBJS = out/csl_util.o out/devices.o out/streams.o out/init.o out/state.o out/hash.o out/wav.o out/csl_types.o out/track.o

out/csl_util.o: src/csl_util.c inc/csl_util.h inc/csl_types.h inc/state.h 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/devices.o: src/devices.c inc/devices.h inc/csl_types.h inc/csl_util.h inc/streams.h inc/init.h inc/state.h inc/errors.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/streams.o: src/streams.c inc/csl_types.h inc/streams.h inc/devices.h inc/csl_util.h inc/init.h inc/state.h inc/wav.h inc/errors.h inc/track.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/init.o: src/init.c inc/init.h inc/errors.h inc/csl_types.h inc/streams.h inc/devices.h inc/state.h inc/wav.h inc/csoundlib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/state.o: src/state.c inc/state.h inc/csoundlib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/hash.o: src/hash.c inc/hash.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/wav.o: src/wav.c inc/wav.h 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/csl_types.o: src/csl_types.c inc/csl_types.h inc/state.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
out/track.o: src/track.c inc/track.h inc/state.h inc/errors.h inc/csl_util.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Target library
TARGET = libcsoundlib.a

# Default rule to build everything
all: outdir $(TARGET)

outdir:
	mkdir -p out

# Rule to create the static library
$(TARGET): $(OBJS)
	ar rcs out/$@ $(OBJS)

# Clean rule to remove object files
clean:
	rm -f $(OBJS) out/*.a out/*.dylib

install:
	mv out/$(TARGET) /usr/local/lib/$(TARGET)

.PHONY: all clean