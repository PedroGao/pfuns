all:
	gcc -DPMALLOC -c pmalloc.c
	gcc -c main.c
	gcc -Wl,--wrap,malloc -o main main.o pmalloc.o
clean:
	rm main main.o pmalloc.o