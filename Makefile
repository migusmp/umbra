# Makefile básico para umbra
# Uso:
#   make        -> compila (solo lo que cambió desde la última vez)
#   make run    -> compila y ejecuta
#   make clean  -> borra los archivos generados

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -I vendor/glad/include $(shell pkg-config --cflags sdl2)
LDFLAGS := $(shell pkg-config --libs sdl2) -ldl

TARGET := umbra

# Fuentes: tu código + glad (que es C, pero g++ lo compila igual)
SRC := src/main.cpp vendor/glad/src/glad.c

# Un .o por cada .cpp/.c, guardados en build/ para no ensuciar el proyecto
OBJ_DIR := build
OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC)))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Regla genérica: cómo construir un .o a partir de un .cpp
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla genérica: cómo construir un .o a partir de un .c (para glad.c)
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
