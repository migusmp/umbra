#pragma once
#include <cstdint>

// Una entidad NO es un objeto con datos ni comportamiento — es solo un
// número. Toda la información real (posición, malla, lo que sea) vive en
// los componentes, indexados por este ID. Esta es la idea central de ECS:
// separar "quién existe" (Entity) de "qué datos tiene" (Component) de
// "qué lógica actúa sobre esos datos" (System).
using Entity = std::uint32_t;
