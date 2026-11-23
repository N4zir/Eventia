#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include <nlohmann/json.hpp>
#include "CertificadoDatabase.h"
#include <string>

class ApiCertificados {
public:
    explicit ApiCertificados(const std::string& connStr);
    void registrarRutas(crow::App<crow::CORSHandler>& app);

private:
    crow::response listarCertificados();
    crow::response obtenerCertificado(int id);
    crow::response crearCertificado(const crow::request& req);
    crow::response actualizarCertificado(const crow::request& req, int id);
    crow::response eliminarCertificado(int id);

    static nlohmann::json certificadoAJson(const Certificado& c);
    static crow::response respuestaJson(int status, const nlohmann::json& body);

    CertificadoDatabase db_;
};
