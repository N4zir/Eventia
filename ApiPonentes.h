#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "PonenteDatabase.h"
#include <string>

class ApiPonentes {
public:
    explicit ApiPonentes(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarPonentes();
    crow::response obtenerPonente(int id);
    crow::response crearPonente(const crow::request& req);
    crow::response actualizarPonente(const crow::request& req, int id);

    static nlohmann::json ponenteAJson(const Ponente& p);
    static crow::response respuestaJson(int status, const nlohmann::json& body);

    PonenteDatabase db_;
};
