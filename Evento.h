#pragma once
#include <string>

// Fila de la tabla Eventos
struct Evento {
    int id = 0;
    std::string nombre;
    std::string fechaInicio; // YYYY-MM-DD
    std::string fechaFin;    // YYYY-MM-DD
};
