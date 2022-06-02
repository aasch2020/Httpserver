CC = clang
CFLAGS = -Wall -Werror -Wextra -pedantic -g

all: httpserver



httpserver: httpserver.o request.o response.o queue.o
	$(CC) -o httpserver httpserver.o request.o response.o queue.o -pthread
sockdem: sockdem.o
	$(CC) -o sockdem sockdem.o
sockdem.o: sockdem.c
	$(CC) $(CFLAGS) -c sockdem.c

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c
requesttest: requesttest.o request.o
	$(CC) -o requesttest requesttest.o request.o

requesttest.o: requesttest.c
	$(CC) $(CFLAGS) -c requesttest.c
queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c
response.o: response.c
	$(CC) $(CFLAGS) -c response.c

request.o: request.c
	$(CC) $(CFLAGS) -c request.c

clean:
	rm -f httpserver *.o

format:
	clang-format -i -style=file *.[ch]
