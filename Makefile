CC ?= cc
CFLAG += -Wall

all:
	$(CC) $(DEFINE) -o uart-loopback $(CFLAG) uart-loopback.c 

clean:
	rm uart-loopback