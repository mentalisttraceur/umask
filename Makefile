c: c/umask

c/umask:
	musl-gcc -std=c89 -pedantic -fPIE -Os -s -Wl,--gc-sections \
            -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 \
            -mfloat-abi=softfp -o umask umask.c
	strip -s umask

clean:
	rm -f umask
