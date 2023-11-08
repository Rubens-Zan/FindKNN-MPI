DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -lmpi -O3
CC = mpic++

.PHONY : all debug clean purge

all : findKNN

findKNN: max-heap.o chrono.o
	$(CC)  $^ $(CFLAGS) -o $@


max-heap.o : max-heap.c
	gcc $(CFLAGS) -c $^

chrono.o : chrono.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) findKNN 