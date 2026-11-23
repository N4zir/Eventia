#pragma once
#include <string>

// Representa un registro de la tabla Charlas.
// EventoId y PonenteId son claves foraneas como enteros.
// Los datos legibles se obtienen mediante JOIN con Eventos y Ponentes, como en Usuario.
struct Charla {
    int id = 0;
    int eventoId = 0;
    std::string eventoNombre;
    std::string sala;
    int ponenteId = 0;
    std::string ponenteNombre;
    std::string ponenteInstitucion;
    std::string titulo;
    std::string horaInicio;
    int cupo = 0;
};
