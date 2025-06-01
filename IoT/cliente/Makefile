LIBS= -lpthread -lm  #Libraries used if needed

CC= aarch64-linux-gnu-gcc

SRCS= cliente.c sensores.c
HDRS= sensores.h
BIN = appCliente
#CFLAGS+= -O2 -march=armv8-a
CFLAGS+= -g -O0 
OBJS= $(SRCS:.c=.o)
#OBJS= $(subst .c, .o, $(SRCS))

all : $(BIN)
$(BIN): $(OBJS)
	@echo [link] $@
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
		
%.o: %.c $(HDRS)
	@echo [Compile] $<
	$(CC) -c $(CFLAGS) $< -o $@
		
clean:
	@rm -f $(OBJS) $(BIN)
