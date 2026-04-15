CC = gcc
CFLAGS = -Wall -g

all: jms_console jms_coord

jms_console: jms_console.c
	$(CC) $(CFLAGS) jms_console.c -o jms_console

jms_coord: jms_coord.c
	$(CC) $(CFLAGS) jms_coord.c -o jms_coord

clean:
	rm -f jms_console jms_coord jms_in jms_out