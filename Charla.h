#pragma once
#include <string>

// Representa un registro de la tabla Charlas.
// EventoId y PonenteId son claves foraneas como enteros, sin relaciones navegacionales.
struct Charla {
    int id = 0;
    int eventoId = 0;
    std::string sala;
    int ponenteId = 0;
    std::string titulo;
    std::string horaInicio;
    int cupo = 0;
};
