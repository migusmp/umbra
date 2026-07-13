#pragma once
#include <cassert>
#include <unordered_map>
#include <vector>

#include "entity.hpp"

// Interfaz sin plantilla: necesaria porque Registry guarda un
// ComponentArray<T> DISTINTO por cada tipo T, pero todos deben poder
// tratarse de forma uniforme cuando se destruye una entidad (sin saber de
// qué tipos concretos tiene componentes esa entidad).
class IComponentArray {
   public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

// Guarda todos los componentes de tipo T en un std::vector CONTIGUO
// (data), sin huecos — importante para que iterar sobre ellos sea
// cache-friendly, la ventaja real de ECS frente a "cada objeto con sus
// propios datos sueltos por ahí".
//
// Como los componentes se pueden añadir/quitar en cualquier orden, hace
// falta un mapa que traduzca Entity -> posición dentro de ese array denso,
// y su inverso, para saber a qué entidad pertenece cada hueco al hacer
// swap-and-pop en remove().
template <typename T>
class ComponentArray : public IComponentArray {
   public:
    void insert(Entity entity, T component) {
        assert(entityToIndex.find(entity) == entityToIndex.end() &&
               "No se puede añadir el mismo componente dos veces a la misma entidad");

        size_t newIndex = data.size();
        entityToIndex[entity] = newIndex;
        indexToEntity[newIndex] = entity;
        data.push_back(std::move(component));
    }

    void remove(Entity entity) {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end())
            return;

        // "swap and pop": en vez de desplazar todos los elementos
        // posteriores (O(n)), se mueve el ÚLTIMO elemento del array al
        // hueco que deja el borrado, y se recorta el final. O(1), pero
        // el orden de los elementos dentro del array deja de tener
        // significado (no importa, porque solo accedemos por Entity, no
        // por posición).
        size_t indexOfRemoved = it->second;
        size_t indexOfLast = data.size() - 1;

        data[indexOfRemoved] = std::move(data[indexOfLast]);

        Entity entityOfLast = indexToEntity[indexOfLast];
        entityToIndex[entityOfLast] = indexOfRemoved;
        indexToEntity[indexOfRemoved] = entityOfLast;

        entityToIndex.erase(entity);
        indexToEntity.erase(indexOfLast);
        data.pop_back();
    }

    bool has(Entity entity) const {
        return entityToIndex.find(entity) != entityToIndex.end();
    }

    T& get(Entity entity) {
        assert(has(entity) && "La entidad no tiene este componente");
        return data[entityToIndex.at(entity)];
    }

    // Llamado por Registry::destroyEntity para cada tipo de componente
    // existente, sin necesidad de saber si esta entidad concreta tiene
    // o no un componente de este tipo — remove() ya comprueba has().
    void entityDestroyed(Entity entity) override {
        if (has(entity))
            remove(entity);
    }

   private:
    std::vector<T> data;
    std::unordered_map<Entity, size_t> entityToIndex;
    std::unordered_map<size_t, Entity> indexToEntity;
};
