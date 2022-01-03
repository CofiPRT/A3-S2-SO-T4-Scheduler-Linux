CC = gcc
CFLAGS = -Wextra -Wall -g

build:
	$(CC) -fPIC $(CFLAGS) -o so_scheduler.o -c so_scheduler.c
	$(CC) -fPIC $(CFLAGS) -o scheduler.o -c scheduler.c
	$(CC) -fPIC $(CFLAGS) -o thread.o -c thread.c
	$(CC) -fPIC $(CFLAGS) -shared -o libscheduler.so so_scheduler.o scheduler.o thread.o

clean:
	rm -f so_scheduler.o scheduler.o thread.o libscheduler.so