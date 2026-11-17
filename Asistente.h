#pragma once
#include <string>

// Representa un registro de la tabla Asistentes.
// No incluye relaciones (usuario, cuenta, inscripciones): esos datos viven en otras tablas.
struct Asistente {
    int id = 0;
    std::string nombre;
    std::string correo;
};
