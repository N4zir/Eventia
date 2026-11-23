#pragma once
#include <string>

// Representa un registro de la tabla Inscripciones.
// AsistenteId y CharlaId son claves foraneas como enteros
struct Inscripcion {
    int id = 0;
    int asistenteId = 0;
    int charlaId = 0;
    std::string fecha;
    bool asistio = false;
};
