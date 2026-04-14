all: jms_console

jms_console: jms_console.c
	gcc -Wall -g jms_console.c -o jms_console

clean:
	rm -f jms_console