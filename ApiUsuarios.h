#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "UsuarioDatabase.h"
#include <string>

class ApiUsuarios {
public:
    explicit ApiUsuarios(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response login(const crow::request& req);
    crow::response listarUsuarios();
    crow::response obtenerUsuario(int id);
    crow::response crearUsuario(const crow::request& req);
    crow::response actualizarUsuario(const crow::request& req, int id);
    crow::response desactivarUsuario(int id);

    static nlohmann::json usuarioAJson(const Usuario& u);
    static crow::response respuestaJson(
        int status,
        const nlohmann::json& body
    );

    UsuarioDatabase db_;
};
