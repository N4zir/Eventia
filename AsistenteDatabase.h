#pragma once
#include "Asistente.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Se lanza cuando un INSERT o UPDATE choca con la restricción UNIQUE de Correo.
class DuplicadoAsistenteException : public std::runtime_error {
public:
    explicit DuplicadoAsistenteException(const std::string& mensaje)
        : std::runtime_error(mensaje) {
    }
};

// Acceso a SQL Server para la tabla Asistentes
class AsistenteDatabase {
public:
    explicit AsistenteDatabase(std::string connectionString);

    // READ
    std::vector<Asistente> listarAsistentes();
    std::optional<Asistente> obtenerAsistente(int id);   // Vacío si no existe

    // CREATE
    int crearAsistente(const Asistente& a);              // Devuelve el Id generado

    // UPDATE
    bool actualizarAsistente(int id, const Asistente& a);

    // DELETE: false si no existe o si tiene registros asociados
    bool eliminarAsistente(int id);

private:
    std::string connStr_;
};
