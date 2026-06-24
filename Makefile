CC      = gcc
CFLAGS  = -Wall -O2 $(shell pkg-config --cflags libcurl)
LDFLAGS = $(shell pkg-config --libs libcurl)

qcli: qcli.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f qcli

.PHONY: clean
