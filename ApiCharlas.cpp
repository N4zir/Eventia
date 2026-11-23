#include "ApiCharlas.h"
#include <exception>

using namespace std;
using json = nlohmann::json;

namespace {

    bool esHoraValida(const string& s) {
        if (s.size() != 5 && s.size() != 8)
            return false;
        if (s[2] != ':')
            return false;
        if (s.size() == 8 && s[5] != ':')
            return false;
        for (size_t i = 0; i < s.size(); ++i) {
            if (i == 2 || i == 5)
                continue;
            if (s[i] < '0' || s[i] > '9')
                return false;
        }
        return true;
    }

    string validarCharla(const Charla& c) {
        if (c.eventoId <= 0)
            return "Debes seleccionar un evento valido.";
        if (c.ponenteId <= 0)
            return "Debes seleccionar un ponente valido.";
        if (c.sala.empty())
            return "La sala es obligatoria.";
        if (c.sala.size() > 40)
            return "La sala no puede superar 40 caracteres.";
        if (c.titulo.empty())
            return "El titulo es obligatorio.";
        if (c.titulo.size() > 300)
            return "El titulo no puede superar 300 caracteres.";
        if (!esHoraValida(c.horaInicio))
            return "La hora de inicio debe tener formato HH:MM o HH:MM:SS.";
        if (c.cupo < 0)
            return "El cupo no puede ser negativo.";
        return "";
    }

} // namespace

ApiCharlas::ApiCharlas(const string& connStr)
    : db_(connStr) {
}

void ApiCharlas::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/charlas").methods("GET"_method)
        ([this]() { return listarCharlas(); });

    CROW_ROUTE(app, "/charlas").methods("POST"_method)
        ([this](const crow::request& req) { return crearCharla(req); });

    CROW_ROUTE(app, "/charlas/<int>").methods("GET"_method)
        ([this](int id) { return obtenerCharla(id); });

    CROW_ROUTE(app, "/charlas/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarCharla(req, id); });

    CROW_ROUTE(app, "/charlas/<int>").methods("DELETE"_method)
        ([this](int id) { return eliminarCharla(id); });
}

// GET /charlas
crow::response ApiCharlas::listarCharlas() {
    try {
        json arr = json::array();
        for (const auto& c : db_.listarCharlas())
            arr.push_back(charlaAJson(c));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /charlas/{id}
crow::response ApiCharlas::obtenerCharla(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto c = db_.obtenerCharla(id);
        if (!c)
            return respuestaJson(404, { {"error", "Charla no encontrada."} });

        return respuestaJson(200, charlaAJson(*c));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /charlas
// {"eventoId":1,"sala":"Sala A","ponenteId":2,"titulo":"Charla","horaInicio":"09:30","cupo":50}
crow::response ApiCharlas::crearCharla(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Charla c;
        c.eventoId = datos.value("eventoId", 0);
        c.sala = datos.value("sala", "");
        c.ponenteId = datos.value("ponenteId", 0);
        c.titulo = datos.value("titulo", "");
        c.horaInicio = datos.value("horaInicio", "");
        c.cupo = datos.value("cupo", 0);

        string error = validarCharla(c);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeEvento(c.eventoId))
            return respuestaJson(400, { {"error", "El evento no existe."} });
        if (!db_.existePonente(c.ponenteId))
            return respuestaJson(400, { {"error", "El ponente no existe."} });

        int id = db_.crearCharla(c);
        auto nuevo = db_.obtenerCharla(id);
        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo la charla, pero no se pudo recuperar."} });

        return respuestaJson(201, charlaAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /charlas/{id}; campos omitidos conservan su valor.
crow::response ApiCharlas::actualizarCharla(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerCharla(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Charla no encontrada."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Charla c = *actual;
        c.eventoId = datos.value("eventoId", actual->eventoId);
        c.sala = datos.value("sala", actual->sala);
        c.ponenteId = datos.value("ponenteId", actual->ponenteId);
        c.titulo = datos.value("titulo", actual->titulo);
        c.horaInicio = datos.value("horaInicio", actual->horaInicio);
        c.cupo = datos.value("cupo", actual->cupo);

        string error = validarCharla(c);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeEvento(c.eventoId))
            return respuestaJson(400, { {"error", "El evento no existe."} });
        if (!db_.existePonente(c.ponenteId))
            return respuestaJson(400, { {"error", "El ponente no existe."} });

        if (!db_.actualizarCharla(id, c))
            return respuestaJson(404, { {"error", "No se pudo actualizar la charla."} });

        auto actualizado = db_.obtenerCharla(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Charla no encontrada despues de actualizar."} });

        return respuestaJson(200, charlaAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// DELETE /charlas/{id}
crow::response ApiCharlas::eliminarCharla(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });
        if (!db_.obtenerCharla(id))
            return respuestaJson(404, { {"error", "Charla no encontrada."} });
        if (!db_.eliminarCharla(id))
            return respuestaJson(409, { {"error", "No se puede eliminar: la charla tiene inscripciones asociadas."} });
        return respuestaJson(200, { {"mensaje", "Charla eliminada correctamente."}, {"id", id} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiCharlas::charlaAJson(const Charla& c) {
    return {
        {"id", c.id},
        {"eventoId", c.eventoId},
        {"eventoNombre", c.eventoNombre},
        {"sala", c.sala},
        {"ponenteId", c.ponenteId},
        {"ponenteNombre", c.ponenteNombre},
        {"ponenteInstitucion", c.ponenteInstitucion},
        {"titulo", c.titulo},
        {"horaInicio", c.horaInicio},
        {"cupo", c.cupo}
    };
}

crow::response ApiCharlas::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
