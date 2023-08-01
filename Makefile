TARGET=main
ALIB=libaspen.a
OBJECTS=sync_data.o
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

$(OBJDIR)%.o: %.cu $(DEPS)
	$(NVCC) $(ARCH) $(COMMON) --compiler-options "$(CFLAGS)" -c $< -o $@

obj:
	mkdir -p obj

test:
	$(CC) test_transfer_rx.c -o test_transfer_rx.out
	$(CC) test_transfer_tx.c -o test_transfer_tx.out

clean:
	rm -rf $(TARGET) $(EXEOBJS) $(OBJS)
