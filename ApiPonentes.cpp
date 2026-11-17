#include "ApiPonentes.h"
#include <exception>

using namespace std;
using json = nlohmann::json;

namespace {

    string validarPonente(const Ponente& p) {
        if (p.nombre.empty())
            return "El nombre es obligatorio.";
        if (p.nombre.size() > 200)
            return "El nombre no puede superar 200 caracteres.";
        if (p.institucion.empty())
            return "La institucion es obligatoria.";
        if (p.institucion.size() > 160)
            return "La institucion no puede superar 160 caracteres.";
        return "";
    }

} // namespace

ApiPonentes::ApiPonentes(const string& connStr)
    : db_(connStr) {
}

void ApiPonentes::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/ponentes").methods("GET"_method)
        ([this]() { return listarPonentes(); });

    CROW_ROUTE(app, "/ponentes").methods("POST"_method)
        ([this](const crow::request& req) { return crearPonente(req); });

    CROW_ROUTE(app, "/ponentes/<int>").methods("GET"_method)
        ([this](int id) { return obtenerPonente(id); });

    CROW_ROUTE(app, "/ponentes/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarPonente(req, id); });
}

// GET /ponentes
crow::response ApiPonentes::listarPonentes() {
    try {
        json arr = json::array();
        for (const auto& p : db_.listarPonentes())
            arr.push_back(ponenteAJson(p));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /ponentes/{id}
crow::response ApiPonentes::obtenerPonente(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto p = db_.obtenerPonente(id);
        if (!p)
            return respuestaJson(404, { {"error", "Ponente no encontrado."} });

        return respuestaJson(200, ponenteAJson(*p));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /ponentes
// {"nombre":"Jane Doe","institucion":"Universidad"}
crow::response ApiPonentes::crearPonente(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Ponente p;
        p.nombre = datos.value("nombre", "");
        p.institucion = datos.value("institucion", "");

        string error = validarPonente(p);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        int id = db_.crearPonente(p);
        auto nuevo = db_.obtenerPonente(id);
        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo el ponente, pero no se pudo recuperar."} });

        return respuestaJson(201, ponenteAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /ponentes/{id}; campos omitidos conservan su valor.
crow::response ApiPonentes::actualizarPonente(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerPonente(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Ponente no encontrado."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Ponente p = *actual;
        p.nombre = datos.value("nombre", actual->nombre);
        p.institucion = datos.value("institucion", actual->institucion);

        string error = validarPonente(p);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.actualizarPonente(id, p))
            return respuestaJson(404, { {"error", "No se pudo actualizar el ponente."} });

        auto actualizado = db_.obtenerPonente(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Ponente no encontrado despues de actualizar."} });

        return respuestaJson(200, ponenteAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiPonentes::ponenteAJson(const Ponente& p) {
    return {
        {"id", p.id},
        {"nombre", p.nombre},
        {"institucion", p.institucion}
    };
}

crow::response ApiPonentes::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
