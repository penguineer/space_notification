# http://stackoverflow.com/questions/2145590/what-is-the-purpose-of-phony-in-a-makefile

DEBUG   = -O3                                                                   
CC      = gcc                                                                   
INCLUDE = -I/usr/local/include                                                  
CFLAGS  = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe                              
                                                                                
LDFLAGS = -L/usr/local/lib                                                      
LDLIBS    = -lwiringPi -lwiringPiDev -lpthread -lm -lmosquitto


.phony: clean

all: ledcontrol

clean:
	rm ledcontrol *.o

ledcontrol: ledcontrol.o
	@$(CC) -o $@ ledcontrol.o $(LDFLAGS) $(LDLIBS) 

ledcontrol.o: ledcontrol.c
	@$(CC) -c ledcontrol.c -o $@

