TARGET=main
ALIB=libconlin.a
OBJECTS=sync_data.o utils.o
AVX2=1
NEON=0
GPU=0
DEBUG=0
SUPPRESS_OUTPUT=1

CC=gcc

OBJDIR=./obj/
VPATH=./src 
DEPS = $(wildcard src/*.h) $(wildcard include/*.h) Makefile

OPTS=-Ofast -march=native -funroll-loops
LDFLAGS=-lm -lgomp
COMMON= -Iinclude/
ifeq ($(DEBUG), 1) 
OPTS=-O0 -g -DDEBUG
endif
CFLAGS= -Wall -fopenmp
ARFLAGS=rcs
ifeq ($(SUPPRESS_OUTPUT), 1) 
OPTS+=-D_SUPPRESS_OUTPUT
endif

OBJS= $(addprefix $(OBJDIR), $(OBJECTS))
EXEOBJSA= $(addsuffix .o, $(TARGET))
EXEOBJS= $(addprefix $(OBJDIR), $(EXEOBJSA))

all: obj $(TARGET)

$(TARGET): $(EXEOBJS) $(ALIB) 
	$(CC) $(COMMON) $(CFLAGS) $(OPTS) $^ -o $@ $(LDFLAGS) $(ALIB)

$(ALIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR): $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) $(OPTS) -c $< -o $@

$(OBJDIR)%.o: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) $(OPTS) -c $< -o $@

obj:
	mkdir -p obj

test: $(ALIB)
	$(CC) $(COMMON) $(CFLAGS) $(OPTS) ./test/test_sync_data.c -o test_sync_data $(LDFLAGS) $(ALIB)

clean:
	rm -rf $(TARGET) $(EXEOBJS) $(OBJS)
