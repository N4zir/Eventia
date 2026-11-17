#pragma once
#include <string>

// Representa un registro de la tabla Eventos.
// Las fechas se manejan como texto en formato ISO (YYYY-MM-DD).
struct Evento {
    int id = 0;
    std::string nombre;
    std::string fechaInicio;
    std::string fechaFin;
};
