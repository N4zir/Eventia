#pragma once
#include "Evento.h"
#include <optional>
#include <string>
#include <vector>

class EventoDatabase {
public:
    explicit EventoDatabase(std::string connectionString);

    std::vector<Evento> listarEventos();
    std::optional<Evento> obtenerEvento(int id);

    int crearEvento(const Evento& evento);
    bool actualizarEvento(int id, const Evento& evento);

private:
    std::string connStr_;
};
