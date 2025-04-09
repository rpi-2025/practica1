
DEBUG = -O2

#CC = gcc 
CC=arm-linux-gnueabihf-gcc
#INCLUDE = -I/usr/local/include
INCLUDE=
CFLAGS = $(DEBUG) -Wall $(INCLUDE) -Winline
#LDFLAGS = -L/usr/local/lib
LDFLAGS=
SRC = sensor.c

OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)

$(BIN):			$(OBJ)
		@echo [link] $@
		$(CC) -o $@ $< $(LDFLAGS) $(LIBS)

.c.o:
		@echo [Compile] $<
		$(CC) -c $(CFLAGS) $< -o $@
		
clean:
		@rm -f $(OBJ) $(BIN)
