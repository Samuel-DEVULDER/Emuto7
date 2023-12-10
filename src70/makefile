obj=monitor.o d6809.o emuto7.o video.o emup1.o emup2.o emup3.o copyto7.o repert.o

inc=-I/usr/local/X11/include/

lib=-L/usr/local/X11/lib

to770:$(obj)
	gcc -o to770 -O $(obj) $(lib) -lX11

monitor.o: monitor.c monitor.h repert.h
	gcc -c -O monitor.c

d6809.o: d6809.c monitor.h
	gcc -c -O d6809.c

emuto7.o: emuto7.c monitor.h emuto7.h video.h
	gcc -c -O emuto7.c

video.o: video.c monitor.h emuto7.h video.h
	gcc -c -O video.c $(inc)

emup1.o: emup1.c monitor.h emuto7.h video.h
	gcc -c -O emup1.c

emup2.o: emup2.c monitor.h emuto7.h
	gcc -c -O emup2.c

emup3.o: emup3.c monitor.h emuto7.h
	gcc -c -O emup3.c

copyto7.o: copyto7.c monitor.h
	gcc -c -O copyto7.c

repert.o: repert.c config.h
	gcc -c -O repert.c
