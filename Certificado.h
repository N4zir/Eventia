#pragma once
#include <string>

// Representa un registro de la tabla Certificados.
// InscripcionId es clave foranea como entero, sin relaciones navegacionales.
struct Certificado {
    int id = 0;
    int inscripcionId = 0;
    std::string codigo;
    std::string fechaEmision;
};
