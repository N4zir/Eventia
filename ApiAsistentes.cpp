#include "ApiAsistentes.h"
#include <exception>

using namespace std;
using json = nlohmann::json;

namespace {

    string validarAsistente(const Asistente& a) {
        if (a.nombre.empty())
            return "El nombre es obligatorio.";
        if (a.nombre.size() > 100)
            return "El nombre no puede superar 200 caracteres.";
        if (a.correo.empty())
            return "El correo es obligatorio.";
        if (a.correo.size() > 100)
            return "El correo no puede superar 200 caracteres.";
        if (a.correo.find('@') == string::npos)
            return "El correo no es valido.";
        return "";
    }

} // namespace

ApiAsistentes::ApiAsistentes(const string& connStr)
    : db_(connStr) {
}

void ApiAsistentes::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/asistentes").methods("GET"_method)
        ([this]() { return listarAsistentes(); });

    CROW_ROUTE(app, "/asistentes").methods("POST"_method)
        ([this](const crow::request& req) { return crearAsistente(req); });

    CROW_ROUTE(app, "/asistentes/<int>").methods("GET"_method)
        ([this](int id) { return obtenerAsistente(id); });

    CROW_ROUTE(app, "/asistentes/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarAsistente(req, id); });
    CROW_ROUTE(app, "/asistentes/<int>").methods("DELETE"_method)([this](int id) { return eliminarAsistente(id); });
}

// GET /asistentes
crow::response ApiAsistentes::listarAsistentes() {
    try {
        json arr = json::array();
        for (const auto& a : db_.listarAsistentes())
            arr.push_back(asistenteAJson(a));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /asistentes/{id}
crow::response ApiAsistentes::obtenerAsistente(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto a = db_.obtenerAsistente(id);
        if (!a)
            return respuestaJson(404, { {"error", "Asistente no encontrado."} });

        return respuestaJson(200, asistenteAJson(*a));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /asistentes
// {"nombre":"Laura Martinez","correo":"laura@correo.com"}
crow::response ApiAsistentes::crearAsistente(const crow::request& req) {
    try {
        json datos = json::parse(req.body);

        Asistente a;
        a.nombre = datos.value("nombre", "");
        a.correo = datos.value("correo", "");

        string error = validarAsistente(a);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        int id = db_.crearAsistente(a);

        return respuestaJson(201, asistenteAJson(Asistente{id, a.nombre, a.correo}));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const DuplicadoAsistenteException& e) {
        return respuestaJson(409, { {"error", e.what()} });
    }
}

// PUT /asistentes/{id}; campos omitidos conservan su valor.
crow::response ApiAsistentes::actualizarAsistente(const crow::request& req, int id) {
    try {
        auto actual = db_.obtenerAsistente(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Asistente no encontrado."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);

        Asistente a = *actual;
        a.nombre = datos.value("nombre", actual->nombre);
        a.correo = datos.value("correo", actual->correo);

        string error = validarAsistente(a);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });
        db_.actualizarAsistente(id, a); 

        return respuestaJson(200, asistenteAJson(a));
    }
    catch (const json::parse_error&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

crow::response ApiAsistentes::eliminarAsistente(int id) {
    try {
        if (!db_.obtenerAsistente(id))
            return respuestaJson(404, {{"error", "Asistente no encontrado."}});
        if (!db_.eliminarAsistente(id))
            return respuestaJson(409, {{"error", "No se puede eliminar: el asistente tiene un usuario o evento asociados."}});
        return respuestaJson(200, {{"mensaje", "Asistente eliminado correctamente."}, {"id", id}});
    } catch (const exception& e) { return respuestaJson(500, {{"error", e.what()}}); }
}

json ApiAsistentes::asistenteAJson(const Asistente& a) {
    return {
        {"id", a.id},
        {"nombre", a.nombre},
        {"correo", a.correo}
    };
}

crow::response ApiAsistentes::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
