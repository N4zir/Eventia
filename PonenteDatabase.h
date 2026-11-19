#pragma once
#include "Ponente.h"
#include <optional>
#include <string>
#include <vector>

// Acceso a SQL Server para la tabla Ponentes
class PonenteDatabase {
public:
    explicit PonenteDatabase(std::string connectionString);

    // READ
    std::vector<Ponente> listarPonentes();
    std::optional<Ponente> obtenerPonente(int id);   // Vacío si no existe

    // CREATE
    int crearPonente(const Ponente& p);             // Devuelve el Id generado

    // UPDATE
    bool actualizarPonente(int id, const Ponente& p);

    // DELETE: false si no existe o si tiene registros asociados
    bool eliminarPonente(int id);

private:
    std::string connStr_;
};
