#pragma once
#include "Usuario.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Se lanza cuando un INSERT o UPDATE choca con una restriccion UNIQUE
// (nombre de usuario repetido o asistente con mas de una cuenta).
class DuplicadoException : public std::runtime_error {
public:
    explicit DuplicadoException(const std::string& mensaje)
        : std::runtime_error(mensaje) {
    }
};

class UsuarioDatabase {
public:
    explicit UsuarioDatabase(std::string connectionString);

std::optional<Usuario> validarLogin(
        const std::string& nombreUsuario,
        const std::string& clave
    );

    std::vector<Usuario> listarUsuarios();
    std::optional<Usuario> obtenerUsuario(int id);

    bool existeAsistente(int asistenteId);

    int crearUsuario(const Usuario& usuario, const std::string& clave, const std::string& rol);
    bool actualizarUsuario(
        int id,
        const Usuario& usuario,
        const std::string& clave,
        const std::string& rol
    );

    bool desactivarUsuario(int id);

private:
    std::string connStr_;
};
