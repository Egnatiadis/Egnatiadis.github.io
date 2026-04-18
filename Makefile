CC = gcc
CFLAGS = -Wall -g

all: jms_coord jms_console jms_pool

jms_coord: jms_coord.c
	$(CC) $(CFLAGS) -o jms_coord jms_coord.c

jms_console: jms_console.c
	$(CC) $(CFLAGS) -o jms_console jms_console.c

jms_pool: jms_pool.c
	$(CC) $(CFLAGS) -o jms_pool jms_pool.c

clean:
	rm -f jms_coord jms_console jms_pool jms_in jms_out pool_in_* pool_out_*
	rm -rf outputs_*