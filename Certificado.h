#pragma once
#include <string>

// Representa un registro de la tabla Certificados.
// InscripcionId es clave foranea como entero.
// Los nombres se obtienen mediante JOIN con Inscripciones, Asistentes y Charlas, como en Usuario.
struct Certificado {
    int id = 0;
    int inscripcionId = 0;
    std::string asistenteNombre;
    std::string charlaTitulo;
    std::string codigo;
    std::string fechaEmision;
};
