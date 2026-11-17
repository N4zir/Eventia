#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "EventoDatabase.h"
#include <string>

// Nombre distinto de ApiEvento (que es el servidor agregador).
class ApiEventosCrud {
public:
    explicit ApiEventosCrud(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarEventos();
    crow::response obtenerEvento(int id);
    crow::response crearEvento(const crow::request& req);
    crow::response actualizarEvento(const crow::request& req, int id);

    static nlohmann::json eventoAJson(const Evento& e);
    static crow::response respuestaJson(int status, const nlohmann::json& body);

    EventoDatabase db_;
};
