LIBS= -lpthread -lm  #Libraries used if needed

CC= aarch64-linux-gnu-gcc

SRC= cliente.c
BIN = $(SRC:.c=)
CFLAGS+= -O2 -march=armv8-a
OBJ = $(SRC:.c=.o)


#CFLAGS+= -g -O0 -Wno-implicit-function-declaration -fpermissive

all : $(BIN)

$(BIN): $(OBJ)
		@echo [link] $@
		$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)
		
%.o: %.c
		@echo [Compile] $<
		$(CC) -c $(CFLAGS) $< -o $@
		
clean:
		@rm -f $(OBJ) $(BIN)
