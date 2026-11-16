#include "ApiUsuarios.h"
#include <exception>

using namespace std;
using json = nlohmann::json;

namespace {

    string validarUsuario(const Usuario& u, const string& clave, bool esNuevo, const string& rol) {
        if (u.asistenteId <= 0)
            return "Debes seleccionar un asistente valido.";
        if (u.nombreUsuario.empty())
            return "El nombre de usuario es obligatorio.";
        if (u.nombreUsuario.size() > 50)
            return "El nombre de usuario no puede superar 50 caracteres.";
        if (esNuevo && clave.empty())
            return "La clave es obligatoria.";
        if (!clave.empty() && clave.size() < 6)
            return "La clave debe tener al menos 6 caracteres.";
        if (clave.size() > 100)
            return "La clave no puede superar 100 caracteres.";
        return "";
    }

} // namespace

ApiUsuarios::ApiUsuarios(const string& connStr)
    : db_(connStr) {
}

void ApiUsuarios::registrarRutas(crow::App<crow::CORSHandler>& app) {
    CROW_ROUTE(app, "/login").methods("POST"_method)
        ([this](const crow::request& req) { return login(req); });

    CROW_ROUTE(app, "/usuarios").methods("GET"_method)
        ([this]() { return listarUsuarios(); });

    CROW_ROUTE(app, "/usuarios").methods("POST"_method)
        ([this](const crow::request& req) { return crearUsuario(req); });

    CROW_ROUTE(app, "/usuarios/<int>").methods("GET"_method)
        ([this](int id) { return obtenerUsuario(id); });

    CROW_ROUTE(app, "/usuarios/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int id) { return actualizarUsuario(req, id); });

    CROW_ROUTE(app, "/usuarios/<int>").methods("DELETE"_method)
        ([this](int id) { return desactivarUsuario(id); });
}

// POST /login
// {"usuario":"usuario1","clave":"clave123"}
crow::response ApiUsuarios::login(const crow::request& req) {
    try {
        std::cout << "Entrando a login()" << std::endl;

        json datos = json::parse(req.body);

        std::cout << "JSON recibido: " << datos.dump() << std::endl;

        string usuario = datos.value("usuario", "");
        string clave = datos.value("clave", "");

        std::cout << "Usuario: " << usuario << std::endl;

        auto encontrado = db_.validarLogin(usuario, clave);

        std::cout << "validarLogin ejecutado" << std::endl;

        if (!encontrado)
            return respuestaJson(401, { {"error", "Usuario o clave incorrectos."} });

        return respuestaJson(200, usuarioAJson(*encontrado));
    }
    catch (const exception& e) {
        std::cerr << "EXCEPCION LOGIN: " << e.what() << std::endl;

        return respuestaJson(500, {
            {"error", e.what()}
            });
    }
}

// GET /usuarios
crow::response ApiUsuarios::listarUsuarios() {
    try {
        json arr = json::array();
        for (const auto& u : db_.listarUsuarios())
            arr.push_back(usuarioAJson(u));
        return respuestaJson(200, arr);
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// GET /usuarios/{id}
crow::response ApiUsuarios::obtenerUsuario(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto u = db_.obtenerUsuario(id);
        if (!u)
            return respuestaJson(404, { {"error", "Usuario no encontrado."} });

        return respuestaJson(200, usuarioAJson(*u));
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// POST /usuarios
// {"clienteId":1,"nombreUsuario":"usuario1","clave":"clave123"}
crow::response ApiUsuarios::crearUsuario(const crow::request& req) {
    try {
        json datos = json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Usuario u;
        u.asistenteId = datos.value("asistenteId", 0);
        u.nombreUsuario = datos.value("nombreUsuario", "");
        string clave = datos.value("clave", "");
        string rol = datos.value("rol", "");

        string error = validarUsuario(u, clave, true, rol);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        int id = db_.crearUsuario(u, clave, rol);
        auto nuevo = db_.obtenerUsuario(id);

        if (!nuevo)
            return respuestaJson(500, { {"error", "Se creo el usuario, pero no se pudo recuperar."} });

        return respuestaJson(201, usuarioAJson(*nuevo));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// PUT /usuarios/{id}; campos omitidos conservan su valor.
crow::response ApiUsuarios::actualizarUsuario(const crow::request& req, int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        auto actual = db_.obtenerUsuario(id);
        if (!actual)
            return respuestaJson(404, { {"error", "Usuario no encontrado."} });

        json datos = req.body.empty() ? json::object() : json::parse(req.body);
        if (!datos.is_object())
            return respuestaJson(400, { {"error", "Se esperaba un objeto JSON."} });

        Usuario u = *actual;
        u.asistenteId = datos.value("clienteId", actual->asistenteId);
        u.nombreUsuario = datos.value("nombreUsuario", actual->nombreUsuario);
        u.activo = datos.value("activo", actual->activo);
        string clave = datos.value("clave", "");
        string rol = datos.value("clave", "");

        string error = validarUsuario(u, clave, false, rol);
        if (!error.empty())
            return respuestaJson(400, { {"error", error} });

        if (!db_.actualizarUsuario(id, u, clave, rol))
            return respuestaJson(404, { {"error", "No se pudo actualizar el usuario."} });

        auto actualizado = db_.obtenerUsuario(id);
        if (!actualizado)
            return respuestaJson(404, { {"error", "Usuario no encontrado despues de actualizar."} });

        return respuestaJson(200, usuarioAJson(*actualizado));
    }
    catch (const json::exception&) {
        return respuestaJson(400, { {"error", "JSON invalido o tipos de datos incorrectos."} });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

// DELETE /usuarios/{id}: baja lógica.
crow::response ApiUsuarios::desactivarUsuario(int id) {
    try {
        if (id <= 0)
            return respuestaJson(400, { {"error", "El ID debe ser mayor que cero."} });

        if (!db_.desactivarUsuario(id))
            return respuestaJson(404, { {"error", "Usuario no encontrado o ya desactivado."} });

        return respuestaJson(200, {
            {"mensaje", "Usuario desactivado correctamente."},
            {"id", id}
            });
    }
    catch (const exception& e) {
        return respuestaJson(500, { {"error", e.what()} });
    }
}

json ApiUsuarios::usuarioAJson(const Usuario& u) {
    return {
        {"id", u.id},
        {"clienteId", u.asistenteId},
        {"clienteNombre", u.asistenteNombre},
        {"nombreUsuario", u.nombreUsuario},
        {"activo", u.activo},
        {"rol", u.rol}
    };
}

crow::response ApiUsuarios::respuestaJson(int status, const json& body) {
    crow::response res(
        status,
        body.dump(-1, ' ', false, json::error_handler_t::replace)
    );
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}
