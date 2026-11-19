#include "ApiPonentes.h"
#include <exception>
using namespace std;
using json = nlohmann::json;

namespace {
string validarPonente(const Ponente& p) {
    if (p.nombre.empty()) return "El nombre es obligatorio.";
    if (p.nombre.size() > 100) return "El nombre no puede superar 100 caracteres.";
    if (p.institucion.empty()) return "La institucion es obligatoria.";
    if (p.institucion.size() > 80) return "La institucion no puede superar 80 caracteres.";
    return "";
}
}

ApiPonentes::ApiPonentes(const string& connStr) : db_(connStr) {}

void ApiPonentes::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/ponentes").methods("GET"_method)([this]() { return listarPonentes(); });
    CROW_ROUTE(app, "/ponentes").methods("POST"_method)([this](const crow::request& req) { return crearPonente(req); });
    CROW_ROUTE(app, "/ponentes/<int>").methods("GET"_method)([this](int id) { return obtenerPonente(id); });
    CROW_ROUTE(app, "/ponentes/<int>").methods("PUT"_method)([this](const crow::request& req, int id) { return actualizarPonente(req, id); });
    CROW_ROUTE(app, "/ponentes/<int>").methods("DELETE"_method)([this](int id) { return eliminarPonente(id); });
}

crow::response ApiPonentes::listarPonentes() {
    try {
        json arr = json::array();
        for (const auto& p : db_.listarPonentes()) arr.push_back(ponenteAJson(p));
        return respuestaJson(200, arr);
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

crow::response ApiPonentes::obtenerPonente(int id) {
    try {
        auto p = db_.obtenerPonente(id);
        if (!p) return respuestaJson(404, {{"error", "Ponente no encontrado."}});
        return respuestaJson(200, ponenteAJson(*p));
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

crow::response ApiPonentes::crearPonente(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        Ponente p;
        p.nombre = datos.value("nombre", "");
        p.institucion = datos.value("institucion", "");
        string error = validarPonente(p);
        if (!error.empty()) return respuestaJson(400, {{"error", error}});
        int id = db_.crearPonente(p);
        return respuestaJson(201, ponenteAJson(Ponente{id, p.nombre, p.institucion}));
    } catch (const json::parse_error&) {
        return respuestaJson(400, {{"error", "El cuerpo no es JSON valido."}});
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

crow::response ApiPonentes::actualizarPonente(const crow::request& req, int id) {
    try {
        auto actual = db_.obtenerPonente(id);
        if (!actual) return respuestaJson(404, {{"error", "Ponente no encontrado."}});
        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        Ponente p = *actual;
        p.nombre = datos.value("nombre", actual->nombre);
        p.institucion = datos.value("institucion", actual->institucion);
        string error = validarPonente(p);
        if (!error.empty()) return respuestaJson(400, {{"error", error}});
        db_.actualizarPonente(id, p);
        return respuestaJson(200, ponenteAJson(p));
    } catch (const json::parse_error&) {
        return respuestaJson(400, {{"error", "El cuerpo no es JSON valido."}});
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

crow::response ApiPonentes::eliminarPonente(int id) {
    try {
        if (!db_.obtenerPonente(id))
            return respuestaJson(404, {{"error", "Ponente no encontrado."}});
        if (!db_.eliminarPonente(id))
            return respuestaJson(409, {{"error", "No se puede eliminar: el ponente tiene charlas asociadas."}});
        return respuestaJson(200, {{"mensaje", "Ponente eliminado correctamente."}, {"id", id}});
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

json ApiPonentes::ponenteAJson(const Ponente& p) {
    return {{"id", p.id}, {"nombre", p.nombre}, {"institucion", p.institucion}};
}

crow::response ApiPonentes::respuestaJson(int status, const json& body) {
    crow::response res(status, body.dump(-1, ' ', false, json::error_handler_t::replace));
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
