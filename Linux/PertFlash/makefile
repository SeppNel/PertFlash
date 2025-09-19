all: *.c
	mkdir -p build
	gcc pertflash.c serial.c -o build/pertFlash `sdl2-config --cflags --libs` -O3 -lpthread