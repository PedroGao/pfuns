all:
	gcc -DPMALLOC -shared -fpic -o pmalloc.so pmalloc.c -ldl
	gcc -o main main.c
	LD_PRELOAD="./pmalloc.so" ./main
clean:
	rm main pmalloc.so