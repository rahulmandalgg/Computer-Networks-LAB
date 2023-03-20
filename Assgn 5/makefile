libmsocket.a: mysocket.o
	ar -rcs libmsocket.a mysocket.o

mysocket.o: mysocket.c mysocket.h
	gcc -c mysocket.c -o mysocket.o -lpthread

clean:
	rm -f mysocket.o libmsocket.a