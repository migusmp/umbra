# Makefile básico para umbra
# Uso:
#   make        -> compila (solo lo que cambió desde la última vez, headers incluidos)
#   make run    -> compila y ejecuta
#   make clean  -> borra los archivos generados

CXX := g++
# -MMD -MP: generan un .d por cada .o con la lista de headers que usó ese
# archivo. Así Make sabe recompilar un .cpp si CAMBIA UN HEADER suyo, no solo
# si cambia el propio .cpp. Sin esto, tocar un .hpp no forzaba recompilación
# de quien lo incluía, y podían quedar .o desincronizados entre sí (el bug
# que causó el segfault de antes).
CXXFLAGS := -std=c++20 -Wall -Wextra -MMD -MP -I vendor/glad/include \
            -I vendor/imgui -I vendor/imgui/backends \
            $(shell pkg-config --cflags sdl2) $(shell pkg-config --cflags assimp)
LDFLAGS := $(shell pkg-config --libs sdl2) $(shell pkg-config --libs assimp) -ldl

TARGET := umbra

# Fuentes: tu código + glad (que es C, pero g++ lo compila igual)
SRC := src/main.cpp src/core/window.cpp src/core/engine.cpp \
       src/renderer/shader.cpp src/renderer/mesh.cpp src/renderer/model.cpp src/renderer/camera.cpp \
       src/ui/debug_ui.cpp \
       vendor/glad/src/glad.c vendor/imgui/imgui.cpp vendor/imgui/imgui_draw.cpp \
       vendor/imgui/imgui_tables.cpp vendor/imgui/imgui_widgets.cpp \
       vendor/imgui/backends/imgui_impl_opengl3.cpp \
       vendor/imgui/backends/imgui_impl_sdl2.cpp

# Un .o por cada .cpp/.c, guardados en build/ para no ensuciar el proyecto
OBJ_DIR := build
OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC)))
DEPS := $(OBJ:.o=.d)

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

# Incluye los .d generados, si existen — esto es lo que le da a Make el
# conocimiento real de "este .o depende de estos headers"
-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
