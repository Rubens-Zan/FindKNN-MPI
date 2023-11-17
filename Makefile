# Variáveis para os compiladores e opções de compilação
CC=mpicc
CFLAGS=-Wall -O3 -Iinclude
LDFLAGS=-lm
DEBUG_FLAGS = -g -DDEBUG

# Nome do executável
EXECUTABLE=knn-mpi

# Caminhos
SRC_DIR=src
INCLUDE_DIR=include
OBJ_DIR=.

# Arquivos de objeto
OBJECTS=$(OBJ_DIR)/chrono.o $(OBJ_DIR)/knn-mpi.o $(OBJ_DIR)/max-heap.o

# Regra padrão
all: $(EXECUTABLE)

# Regra para o executável
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm -f $(OBJECTS)

# Regra para arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

# Inclui dependências de arquivos de cabeçalho
$(OBJ_DIR)/chrono.o: $(INCLUDE_DIR)/chrono.h
$(OBJ_DIR)/knn-mpi.o: $(INCLUDE_DIR)/knn-mpi.h $(INCLUDE_DIR)/max-heap.h
$(OBJ_DIR)/max-heap.o: $(INCLUDE_DIR)/max-heap.h

debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

# Regra para limpar os arquivos compilados
clean:
	rm -f $(EXECUTABLE) *.out

purge:
	rm -f $(EXECUTABLE) *.out

# Evita conflito com arquivos de mesmo nome que as regras
.PHONY: all clean purge
