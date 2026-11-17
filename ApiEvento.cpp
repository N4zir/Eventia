#include "ApiEvento.h"

using namespace std;

ApiEvento::ApiEvento(const string& connStr)
/* : usuarios_(connStr), clientes_(connStr), vehiculos_(connStr),
 mecanicos_(connStr), servicios_(connStr), ordenes_(connStr),
 ordenServicios_(connStr), facturas_(connStr) {*/
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
    //clientes_.registrarRutas(app_);
    /* vehiculos_.registrarRutas(app_);
     mecanicos_.registrarRutas(app_);
     servicios_.registrarRutas(app_);
     ordenes_.registrarRutas(app_);
     ordenServicios_.registrarRutas(app_);
     facturas_.registrarRutas(app_);*/
}