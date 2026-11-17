#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "AsistenteDatabase.h"
#include <string>

class ApiAsistentes {
public:
    explicit ApiAsistentes(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarAsistentes();
    crow::response obtenerAsistente(int id);
    crow::response crearAsistente(const crow::request& req);
    crow::response actualizarAsistente(const crow::request& req, int id);

    static nlohmann::json asistenteAJson(const Asistente& a);
    static crow::response respuestaJson(
        int status,
        const nlohmann::json& body
    );

    AsistenteDatabase db_;
};
