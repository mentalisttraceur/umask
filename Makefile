default:
	gcc -std=c89 -pedantic -fPIE -Os -o umaskexec umaskexec.c
	strip umaskexec

clean:
	rm -f umaskexec
