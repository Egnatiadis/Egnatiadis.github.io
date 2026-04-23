CC = gcc
CFLAGS = -Wall -g -D_POSIX_C_SOURCE=200809L

all: jms_coord jms_console jms_pool

jms_coord: jms_coord.c
	$(CC) $(CFLAGS) -o jms_coord jms_coord.c

jms_console: jms_console.c
	$(CC) $(CFLAGS) -o jms_console jms_console.c

jms_pool: jms_pool.c
	$(CC) $(CFLAGS) -o jms_pool jms_pool.c

clean:
	rm -f jms_coord jms_console jms_pool jms_in jms_out p*_in p*_out