#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "InscripcionDatabase.h"
#include <string>

class ApiInscripciones {
public:
    explicit ApiInscripciones(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarInscripciones();
    crow::response obtenerInscripcion(int id);
    crow::response crearInscripcion(const crow::request& req);
    crow::response actualizarInscripcion(const crow::request& req, int id);

    static nlohmann::json inscripcionAJson(const Inscripcion& i);
    static crow::response respuestaJson(int status, const nlohmann::json& body);

    InscripcionDatabase db_;
};
