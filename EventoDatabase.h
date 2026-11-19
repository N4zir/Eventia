#pragma once
#include "Evento.h"
#include <optional>
#include <string>
#include <vector>

// Acceso a SQL Server para la tabla Eventos
class EventoDatabase {
public:
    explicit EventoDatabase(std::string connectionString);

    // READ
    std::vector<Evento> listarEventos();
    std::optional<Evento> obtenerEvento(int id);   // Vacío si no existe

    // CREATE
    int crearEvento(const Evento& e);             // Devuelve el Id generado

    // UPDATE
    bool actualizarEvento(int id, const Evento& e);

    // DELETE: false si no existe o si tiene registros asociados
    bool eliminarEvento(int id);

private:
    std::string connStr_;
};
