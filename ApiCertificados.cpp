#include "ApiCertificados.h"
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

    string validarCertificado(const Certificado& c) {
        if (c.inscripcionId <= 0)
            return "Debes seleccionar una inscripcion valida.";
        if (c.codigo.empty())
            return "El codigo es obligatorio.";
        if (c.codigo.size() > 40)
            return "El codigo no puede superar 40 caracteres.";
        if (!esFechaValida(c.fechaEmision))
            return "La fecha de emision debe tener formato YYYY-MM-DD.";
        return "";
    }

} // namespace

ApiCertificados::ApiCertificados(const string& connStr)
    : db_(connStr) {
}

void ApiCertificados::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/certificados").methods("GET"_method)
        ([this]() { return listarCertificados(); });

    CROW_ROUTE(app, "/certificados").methods("POST"_method)
        ([this](const crow::request& req) { return crearCertificado(req); });

    CROW_ROUTE(app, "/certificados/<int>").methods("GET"_method)
        ([this](int id) { return obtenerCertificado(id); });

    CROW_ROUTE(app, "/certificados/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarCertificado(req, id); });
}

// GET /certificados
crow::response ApiCertificados::listarCertificados() {
    try {
        json arr = json::array();
        for (const auto& c : db_.listarCertificados())
            arr.push_back(certificadoAJson(c));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /certificados/{id}
crow::response ApiCertificados::obtenerCertificado(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto c = db_.obtenerCertificado(id);
        if (!c)
            return respuestaJson(404, { {"error", "Certificado no encontrado."} });

        return respuestaJson(200, certificadoAJson(*c));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /certificados
// {"inscripcionId":9,"codigo":"CERT-2026-001","fechaEmision":"2026-05-12"}
crow::response ApiCertificados::crearCertificado(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Certificado c;
        c.inscripcionId = datos.value("inscripcionId", 0);
        c.codigo = datos.value("codigo", "");
        c.fechaEmision = datos.value("fechaEmision", "");

        string error = validarCertificado(c);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeInscripcion(c.inscripcionId))
            return respuestaJson(400, { {"error", "La inscripcion no existe."} });

        int id = db_.crearCertificado(c);
        auto nuevo = db_.obtenerCertificado(id);
        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo el certificado, pero no se pudo recuperar."} });

        return respuestaJson(201, certificadoAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const DuplicadoCertificadoException& e) {
        return respuestaJson(409, { {"error", e.what()} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /certificados/{id}; campos omitidos conservan su valor.
crow::response ApiCertificados::actualizarCertificado(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerCertificado(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Certificado no encontrado."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Certificado c = *actual;
        c.inscripcionId = datos.value("inscripcionId", actual->inscripcionId);
        c.codigo = datos.value("codigo", actual->codigo);
        c.fechaEmision = datos.value("fechaEmision", actual->fechaEmision);

        string error = validarCertificado(c);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.existeInscripcion(c.inscripcionId))
            return respuestaJson(400, { {"error", "La inscripcion no existe."} });

        if (!db_.actualizarCertificado(id, c))
            return respuestaJson(404, { {"error", "No se pudo actualizar el certificado."} });

        auto actualizado = db_.obtenerCertificado(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Certificado no encontrado despues de actualizar."} });

        return respuestaJson(200, certificadoAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const DuplicadoCertificadoException& e) {
        return respuestaJson(409, { {"error", e.what()} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiCertificados::certificadoAJson(const Certificado& c) {
    return {
        {"id", c.id},
        {"inscripcionId", c.inscripcionId},
        {"codigo", c.codigo},
        {"fechaEmision", c.fechaEmision}
    };
}

crow::response ApiCertificados::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
