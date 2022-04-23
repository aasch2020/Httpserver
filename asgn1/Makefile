CC = clang
CFLAGS = -Wall -pedantic -Werror -Wextra

all: httpserver

httpserver: httpserver.o request.o
	$(CC) -o httpserver httpserver.o request.o

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c

request.o: request.c
	$(CC) $(CFLAGS) -c request.c

clean:
	rm -f httpserver httpserver.o request.o

format:
	clang-format -i -style=file *.[ch]
