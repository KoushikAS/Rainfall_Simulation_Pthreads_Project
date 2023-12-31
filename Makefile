CC=gcc
CFLAGS=-Wall -g


all: rainfall_seq rainfall_pt

rainfall_seq: rainfall_seq.c
	$(CC) $(CFLAGS) -o rainfall_seq rainfall_seq.c

rainfall_pt: rainfall_pt.c
	$(CC) $(CFLAGS) -o rainfall_pt rainfall_pt.c

clean:
	rm -f rainfall_seq
