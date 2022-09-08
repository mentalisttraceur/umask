default:
	gcc -std=c89 -pedantic -fPIE -Os \
	    -Wno-overlength-strings -o umaskexec umaskexec.c
	strip umaskexec

clean:
	rm -f umaskexec
