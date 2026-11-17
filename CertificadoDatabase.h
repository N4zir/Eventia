#pragma once
#include "Certificado.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Se lanza cuando choca con UNIQUE de InscripcionId o de Codigo.
class DuplicadoCertificadoException : public std::runtime_error {
public:
    explicit DuplicadoCertificadoException(const std::string& mensaje)
        : std::runtime_error(mensaje) {
    }
};

class CertificadoDatabase {
public:
    explicit CertificadoDatabase(std::string connectionString);

    std::vector<Certificado> listarCertificados();
    std::optional<Certificado> obtenerCertificado(int id);

    bool existeInscripcion(int inscripcionId);

    int crearCertificado(const Certificado& certificado);
    bool actualizarCertificado(int id, const Certificado& certificado);

private:
    std::string connStr_;
};
