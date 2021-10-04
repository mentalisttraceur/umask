default:
	gcc -std=c89 -pedantic \
	    -fPIE -Os -s -Wl,--gc-sections \
            -o umaskexec umaskexec.c
	strip -s umaskexec

clean:
	rm -f umaskexec
