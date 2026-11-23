#pragma once
#include "Inscripcion.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Se lanza cuando una inscripcion repetiria la combinacion AsistenteId + CharlaId.
class DuplicadoInscripcionException : public std::runtime_error {
public:
    explicit DuplicadoInscripcionException(const std::string& mensaje)
        : std::runtime_error(mensaje) {
    }
};

class InscripcionDatabase {
public:
    explicit InscripcionDatabase(std::string connectionString);

    std::vector<Inscripcion> listarInscripciones();
    std::optional<Inscripcion> obtenerInscripcion(int id);

    bool existeAsistente(int asistenteId);
    bool existeCharla(int charlaId);

    int crearInscripcion(const Inscripcion& inscripcion);
    bool actualizarInscripcion(int id, const Inscripcion& inscripcion);
    bool eliminarInscripcion(int id);

private:
    std::string connStr_;
};
