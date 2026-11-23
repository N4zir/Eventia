#include "ApiInscripciones.h"
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

    string validarInscripcion(const Inscripcion& i) {
        if (i.asistenteId <= 0)
            return "Debes seleccionar un asistente valido.";
        if (i.charlaId <= 0)
            return "Debes seleccionar una charla valida.";
        if (!esFechaValida(i.fecha))
            return "La fecha debe tener formato YYYY-MM-DD.";
        return "";
    }

} // namespace

ApiInscripciones::ApiInscripciones(const string& connStr)
    : db_(connStr) {
}

void ApiInscripciones::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/inscripciones").methods("GET"_method)
        ([this]() { return listarInscripciones(); });

    CROW_ROUTE(app, "/inscripciones").methods("POST"_method)
        ([this](const crow::request& req) { return crearInscripcion(req); });

    CROW_ROUTE(app, "/inscripciones/<int>").methods("GET"_method)
        ([this](int id) { return obtenerInscripcion(id); });

    CROW_ROUTE(app, "/inscripciones/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarInscripcion(req, id); });

    CROW_ROUTE(app, "/inscripciones/<int>").methods("DELETE"_method)
        ([this](int id) { return eliminarInscripcion(id); });
}

// GET /inscripciones
crow::response ApiInscripciones::listarInscripciones() {
    try {
        json arr = json::array();
        for (const auto& i : db_.listarInscripciones())
            arr.push_back(inscripcionAJson(i));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /inscripciones/{id}
crow::response ApiInscripciones::obtenerInscripcion(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto i = db_.obtenerInscripcion(id);
        if (!i)
            return respuestaJson(404, { {"error", "Inscripcion no encontrada."} });

        return respuestaJson(200, inscripcionAJson(*i));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /inscripciones
// {"asistenteId":1,"charlaId":2,"fecha":"2026-05-10","asistio":false}
crow::response ApiInscripciones::crearInscripcion(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Inscripcion i;
        i.asistenteId = datos.value("asistenteId", 0);
        i.charlaId = datos.value("charlaId", 0);
        i.fecha = datos.value("fecha", "");
        i.asistio = datos.value("asistio", false);

        string error = validarInscripcion(i);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeAsistente(i.asistenteId))
            return respuestaJson(400, { {"error", "El asistente no existe."} });
        if (!db_.existeCharla(i.charlaId))
            return respuestaJson(400, { {"error", "La charla no existe."} });

        int id = db_.crearInscripcion(i);
        auto nuevo = db_.obtenerInscripcion(id);
        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo la inscripcion, pero no se pudo recuperar."} });

        return respuestaJson(201, inscripcionAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const DuplicadoInscripcionException& e) {
        return respuestaJson(409, { {"error", e.what()} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /inscripciones/{id}; campos omitidos conservan su valor.
crow::response ApiInscripciones::actualizarInscripcion(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerInscripcion(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Inscripcion no encontrada."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Inscripcion i = *actual;
        i.asistenteId = datos.value("asistenteId", actual->asistenteId);
        i.charlaId = datos.value("charlaId", actual->charlaId);
        i.fecha = datos.value("fecha", actual->fecha);
        i.asistio = datos.value("asistio", actual->asistio);

        string error = validarInscripcion(i);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeAsistente(i.asistenteId))
            return respuestaJson(400, { {"error", "El asistente no existe."} });
        if (!db_.existeCharla(i.charlaId))
            return respuestaJson(400, { {"error", "La charla no existe."} });

        if (!db_.actualizarInscripcion(id, i))
            return respuestaJson(404, { {"error", "No se pudo actualizar la inscripcion."} });

        auto actualizado = db_.obtenerInscripcion(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Inscripcion no encontrada despues de actualizar."} });

        return respuestaJson(200, inscripcionAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const DuplicadoInscripcionException& e) {
        return respuestaJson(409, { {"error", e.what()} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// DELETE /inscripciones/{id}
crow::response ApiInscripciones::eliminarInscripcion(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });
        if (!db_.obtenerInscripcion(id))
            return respuestaJson(404, { {"error", "Inscripcion no encontrada."} });
        if (!db_.eliminarInscripcion(id))
            return respuestaJson(409, { {"error", "No se puede eliminar: la inscripcion tiene un certificado asociado."} });
        return respuestaJson(200, { {"mensaje", "Inscripcion eliminada correctamente."}, {"id", id} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiInscripciones::inscripcionAJson(const Inscripcion& i) {
    return {
        {"id", i.id},
        {"asistenteId", i.asistenteId},
        {"asistenteNombre", i.asistenteNombre},
        {"charlaId", i.charlaId},
        {"charlaTitulo", i.charlaTitulo},
        {"fecha", i.fecha},
        {"asistio", i.asistio}
    };
}

crow::response ApiInscripciones::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
