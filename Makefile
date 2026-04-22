# Ορισμός μεταβλητών
CC = gcc
CFLAGS = -Wall -Wextra -g

# Τα τελικά εκτελέσιμα που θα παραχθούν
TARGETS = jms_coord jms_console jms_pool

# Προεπιλεγμένος στόχος: Φτιάξε όλα τα εκτελέσιμα
all: $(TARGETS)

# Compilation για τον Συντονιστή
jms_coord: jms_coord.c
	$(CC) $(CFLAGS) -o jms_coord jms_coord.c

# Compilation για την Κονσόλα
jms_console: jms_console.c
	$(CC) $(CFLAGS) -o jms_console jms_console.c

jms_pool: jms_pool.c
	$(CC) $(CFLAGS) -o jms_pool jms_pool.c

clean:
	rm -f $(TARGETS) jms_in jms_out
	rm -rf outputs_*
	
.PHONY: all clean