#include "ApiEvento.h"

using namespace std;

ApiEvento::ApiEvento(const string& connStr)
    : usuarios_(connStr), asistentes_(connStr), eventos_(connStr),
      ponentes_(connStr), charlas_(connStr), inscripciones_(connStr),
      certificados_(connStr) {
    configurarCors();
    registrarRutas();
}

void ApiEvento::iniciar(int puerto) {
    app_.port(puerto).multithreaded().run();
}

void ApiEvento::configurarCors() {
    auto& cors = app_.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .headers("Content-Type");
}

void ApiEvento::registrarRutas() {
    usuarios_.registrarRutas(app_);
    asistentes_.registrarRutas(app_);
    eventos_.registrarRutas(app_);
    ponentes_.registrarRutas(app_);
    charlas_.registrarRutas(app_);
    inscripciones_.registrarRutas(app_);
    certificados_.registrarRutas(app_);
}