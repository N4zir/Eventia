#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "CharlaDatabase.h"
#include <string>

class ApiCharlas {
public:
    explicit ApiCharlas(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarCharlas();
    crow::response obtenerCharla(int id);
    crow::response crearCharla(const crow::request& req);
    crow::response actualizarCharla(const crow::request& req, int id);

    static nlohmann::json charlaAJson(const Charla& c);
    static crow::response respuestaJson(int status, const nlohmann::json& body);

    CharlaDatabase db_;
};
