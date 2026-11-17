#include "ApiEventosCrud.h"
#include <exception>

using namespace std;
using json = nlohmann::json;

namespace {

    bool esFechaValida(const string& s) {
        if (s.size() != 10)
            return false;
        if (s[4] != '-' || s[7] != '-')
            return false;
        for (size_t i = 0; i < s.size(); ++i) {
            if (i == 4 || i == 7)
                continue;
            if (s[i] < '0' || s[i] > '9')
                return false;
        }
        return true;
    }

    string validarEvento(const Evento& e) {
        if (e.nombre.empty())
            return "El nombre es obligatorio.";
        if (e.nombre.size() > 240)
            return "El nombre no puede superar 240 caracteres.";
        if (!esFechaValida(e.fechaInicio))
            return "La fecha de inicio debe tener formato YYYY-MM-DD.";
        if (!esFechaValida(e.fechaFin))
            return "La fecha de fin debe tener formato YYYY-MM-DD.";
        if (e.fechaInicio > e.fechaFin)
            return "La fecha de inicio no puede ser posterior a la fecha de fin.";
        return "";
    }

} // namespace

ApiEventosCrud::ApiEventosCrud(const string& connStr)
    : db_(connStr) {
}

void ApiEventosCrud::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/eventos").methods("GET"_method)
        ([this]() { return listarEventos(); });

    CROW_ROUTE(app, "/eventos").methods("POST"_method)
        ([this](const crow::request& req) { return crearEvento(req); });

    CROW_ROUTE(app, "/eventos/<int>").methods("GET"_method)
        ([this](int id) { return obtenerEvento(id); });

    CROW_ROUTE(app, "/eventos/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarEvento(req, id); });
}

// GET /eventos
crow::response ApiEventosCrud::listarEventos() {
    try {
        json arr = json::array();
        for (const auto& e : db_.listarEventos())
            arr.push_back(eventoAJson(e));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /eventos/{id}
crow::response ApiEventosCrud::obtenerEvento(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto e = db_.obtenerEvento(id);
        if (!e)
            return respuestaJson(404, { {"error", "Evento no encontrado."} });

        return respuestaJson(200, eventoAJson(*e));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /eventos
// {"nombre":"Eventia 2026","fechaInicio":"2026-05-10","fechaFin":"2026-05-12"}
crow::response ApiEventosCrud::crearEvento(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Evento e;
        e.nombre = datos.value("nombre", "");
        e.fechaInicio = datos.value("fechaInicio", "");
        e.fechaFin = datos.value("fechaFin", "");

        string error = validarEvento(e);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        int id = db_.crearEvento(e);
        auto nuevo = db_.obtenerEvento(id);
        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo el evento, pero no se pudo recuperar."} });

        return respuestaJson(201, eventoAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /eventos/{id}; campos omitidos conservan su valor.
crow::response ApiEventosCrud::actualizarEvento(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerEvento(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Evento no encontrado."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Evento e = *actual;
        e.nombre = datos.value("nombre", actual->nombre);
        e.fechaInicio = datos.value("fechaInicio", actual->fechaInicio);
        e.fechaFin = datos.value("fechaFin", actual->fechaFin);

        string error = validarEvento(e);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.actualizarEvento(id, e))
            return respuestaJson(404, { {"error", "No se pudo actualizar el evento."} });

        auto actualizado = db_.obtenerEvento(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Evento no encontrado despues de actualizar."} });

        return respuestaJson(200, eventoAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiEventosCrud::eventoAJson(const Evento& e) {
    return {
        {"id", e.id},
        {"nombre", e.nombre},
        {"fechaInicio", e.fechaInicio},
        {"fechaFin", e.fechaFin}
    };
}

crow::response ApiEventosCrud::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
