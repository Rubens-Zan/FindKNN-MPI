# Variáveis para os compiladores e opções de compilação
CC=mpicc
CFLAGS=-Wall -O3
LDFLAGS=-lm

# Nome do executável
EXECUTABLE=knn-mpi

# Arquivos de objeto
OBJECTS=chrono.o knn-mpi.o max-heap.o

# Regra padrão
all: $(EXECUTABLE)

# Regra para o executável
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Regra para arquivos objeto
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Inclui dependências de arquivos de cabeçalho
chrono.o: chrono.h
knn-mpi.o: knn-mpi.h max-heap.h
max-heap.o: max-heap.h

# Regra para limpar os arquivos compilados
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

# Evita conflito com arquivos de mesmo nome que as regras
.PHONY: all clean
