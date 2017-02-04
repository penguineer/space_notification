# http://stackoverflow.com/questions/2145590/what-is-the-purpose-of-phony-in-a-makefile

DEBUG   = -O3                                                                   
CC      = gcc                                                                   
INCLUDE = -I/usr/local/include                                                  
CFLAGS  = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe                              
                                                                                
LDFLAGS = -L/usr/local/lib                                                      
LDLIBS    = -lwiringPi -lwiringPiDev -lpthread -lm -lmosquitto


.phony: clean

all: statusswitch

clean:
	rm statusswitch *.o

statusswitch: statusswitch.o
	@$(CC) -o $@ statusswitch.o $(LDFLAGS) $(LDLIBS) 

statusswitch.o: statusswitch.c
	@$(CC) -c statusswitch.c -o $@
