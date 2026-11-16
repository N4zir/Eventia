#pragma once
#include "Usuario.h"
#include <optional>
#include <string>
#include <vector>

class UsuarioDatabase {
public:
    explicit UsuarioDatabase(std::string connectionString);

    std::optional<Usuario> validarLogin(
        const std::string& nombreUsuario,
        const std::string& clave
    );

    std::vector<Usuario> listarUsuarios();
    std::optional<Usuario> obtenerUsuario(int id);

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
