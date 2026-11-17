#pragma once
#include "Asistente.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Se lanza cuando un INSERT o UPDATE choca con la restriccion UNIQUE de Correo.
class DuplicadoAsistenteException : public std::runtime_error {
public:
    explicit DuplicadoAsistenteException(const std::string& mensaje)
        : std::runtime_error(mensaje) {
    }
};

class AsistenteDatabase {
public:
    explicit AsistenteDatabase(std::string connectionString);

    std::vector<Asistente> listarAsistentes();
    std::optional<Asistente> obtenerAsistente(int id);

    int crearAsistente(const Asistente& asistente);
    bool actualizarAsistente(int id, const Asistente& asistente);

private:
    std::string connStr_;
};
