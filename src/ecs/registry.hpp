#pragma once
#include <algorithm>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "component_array.hpp"
#include "entity.hpp"

// Registry es el punto de entrada único a todo el ECS. Es TEMPLATE-heavy
// (addComponent<T>, getComponent<T>...) y por eso vive entero en el
// header, sin .cpp — el compilador necesita ver el código completo de
// cada plantilla en cada sitio donde se usa con un tipo concreto.
class Registry {
   public:
    Entity createEntity() {
        Entity id = nextEntity++;
        livingEntities.push_back(id);
        return id;
    }

    void destroyEntity(Entity entity) {
        // Se avisa a TODOS los ComponentArray existentes, no solo a los
        // tipos que esta entidad realmente tiene — cada array ya sabe
        // ignorar la llamada si esa entidad no tenía un componente suyo.
        for (auto& [type, array] : componentArrays) {
            array->entityDestroyed(entity);
        }
        livingEntities.erase(std::remove(livingEntities.begin(), livingEntities.end(), entity),
                             livingEntities.end());
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>().insert(entity, std::move(component));
    }

    template <typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>().remove(entity);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        return getComponentArray<T>().get(entity);
    }

    template <typename T>
    bool hasComponent(Entity entity) {
        return getComponentArray<T>().has(entity);
    }

    // Devuelve las entidades vivas que tienen TODOS los tipos de
    // componente indicados. Es la función que usarán los "sistemas": por
    // ejemplo, un sistema de renderizado querrá
    // view<TransformComponent, MeshRendererComponent>().
    //
    // Implementación simple a propósito: recorre TODAS las entidades vivas
    // y comprueba cada componente pedido. No es la forma más rápida
    // posible (una sparse-set real evitaría recorrer entidades que no
    // encajan), pero es clara, y de sobra para las cientos/pocos miles de
    // entidades que va a tener este motor en su fase de aprendizaje.
    template <typename... Components>
    std::vector<Entity> view() {
        std::vector<Entity> result;
        for (Entity entity : livingEntities) {
            if ((hasComponent<Components>(entity) && ...)) {
                result.push_back(entity);
            }
        }
        return result;
    }

   private:
    template <typename T>
    ComponentArray<T>& getComponentArray() {
        // std::type_index identifica el tipo T en tiempo de ejecución —
        // es cómo Registry sabe "qué ComponentArray<T> concreto"
        // corresponde a cada tipo, sin tener una lista fija de tipos
        // conocidos de antemano.
        std::type_index typeIndex(typeid(T));

        auto it = componentArrays.find(typeIndex);
        if (it == componentArrays.end()) {
            // Primera vez que se pide un componente de este tipo: se crea
            // su ComponentArray<T> sobre la marcha.
            auto array = std::make_unique<ComponentArray<T>>();
            ComponentArray<T>* ptr = array.get();
            componentArrays[typeIndex] = std::move(array);
            return *ptr;
        }
        return *static_cast<ComponentArray<T>*>(it->second.get());
    }

    Entity nextEntity = 0;
    std::vector<Entity> livingEntities;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> componentArrays;
};
