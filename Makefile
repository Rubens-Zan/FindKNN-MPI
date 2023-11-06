DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -lm -lpthread -O3 -lmpi
CC = mpic++

.PHONY : all debug clean purge

all : findKNN

findKNN: max-heap.o chrono.o
	gcc -o $@ $^ $(CFLAGS)


max-heap.o : max-heap.c
	gcc $(CFLAGS) -c $^

chrono.o : chrono.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) findKLeast 