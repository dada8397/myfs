CC = gcc

main: main.c myfs.so
	$(CC) main.c -ldl -g -o main

myfs.so: myfs.c myfs.h
	$(CC) -fPIC -shared -g myfs.c -o myfs.so

clean:
	@rm -f main *.o *.so ./myfs/* ./myfs/*.*
