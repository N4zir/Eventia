#include "UsuarioDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>
#include <iostream>

using namespace std;

namespace {

    // Columnas:
    // 1 = Id
    // 2 = AsistenteId
    // 3 = Asistente.Nombre
    // 4 = NombreUsuario
    // 5 = Rol
    // 6 = Activo
    const string SQL_SELECT_USUARIOS =
        "SELECT u.Id, u.AsistenteId, c.Nombre, "
        "u.NombreUsuario, u.Rol, u.Activo "
        "FROM Usuarios AS u "
        "INNER JOIN Asistentes AS c ON c.Id = u.AsistenteId";

    Usuario leerUsuario(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        SQLINTEGER asistenteId = 0;
        SQLCHAR activo = 0;

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID del usuario.");
        }

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 2, SQL_C_SLONG, &asistenteId, sizeof(asistenteId), nullptr))) {
            throw runtime_error("No se pudo leer el ID del asistente.");
        }

        Usuario u;
        u.id = static_cast<int>(id);
        u.asistenteId = static_cast<int>(asistenteId);
        u.asistenteNombre = leerTexto(stmt, 3);
        u.nombreUsuario = leerTexto(stmt, 4);
        u.rol = leerTexto(stmt, 5);

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 6, SQL_C_BIT, &activo, sizeof(activo), nullptr))) {
            throw runtime_error("No se pudo leer el estado del usuario.");
        }

        u.activo = (activo != 0);
        return u;
    }

} // namespace

UsuarioDatabase::UsuarioDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LOGIN
optional<Usuario> UsuarioDatabase::validarLogin(
    const string& nombreUsuario,
    const string& clave
) {
    Conexion con(connStr_);

    wstring usuarioW = aWide(nombreUsuario);
    wstring claveW = aWide(clave);
    SQLLEN longitudUsuario = 0;
    SQLLEN longitudClave = 0;

    bindTexto(con.stmt, 1, usuarioW, longitudUsuario, 50);
    bindTexto(con.stmt, 2, claveW, longitudClave, 100);

    const string sql = SQL_SELECT_USUARIOS +
        " WHERE u.NombreUsuario = ? "
        "AND u.Clave = CONVERT(NVARCHAR(64), "
        "HASHBYTES('SHA2_256', CAST(? AS NVARCHAR(100))), 2) "
        "AND u.Activo = 1";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
    {
        SQLWCHAR state[10];
        SQLWCHAR message[512];
        SQLINTEGER nativeError;
        SQLSMALLINT length;

        SQLGetDiagRecW(
            SQL_HANDLE_STMT,
            con.stmt,
            1,
            state,
            &nativeError,
            message,
            sizeof(message) / sizeof(SQLWCHAR),
            &length
        );

        std::wcerr << L"SQLSTATE: " << state << std::endl;
        std::wcerr << L"MENSAJE: " << message << std::endl;

        throw runtime_error("Error al validar el inicio de sesion.");
    }


    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el usuario autenticado.");

    return leerUsuario(con.stmt);
}

// LISTAR
vector<Usuario> UsuarioDatabase::listarUsuarios() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_USUARIOS + " ORDER BY u.Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar usuarios.");

    vector<Usuario> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de usuarios.");
        lista.push_back(leerUsuario(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Usuario> UsuarioDatabase::obtenerUsuario(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_USUARIOS + " WHERE u.Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el usuario.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el usuario.");

    return leerUsuario(con.stmt);
}

// CREAR
int UsuarioDatabase::crearUsuario(const Usuario& u, const string& clave, const string& rol) {
    Conexion con(connStr_);

    SQLINTEGER asistenteId = static_cast<SQLINTEGER>(u.asistenteId);
    wstring nombreUsuario = aWide(u.nombreUsuario);
    wstring claveW = aWide(clave);
    wstring rolW = aWide(rol);
    SQLLEN longitudUsuario = 0;
    SQLLEN longitudClave = 0;
    SQLLEN longitudRol = 0;

    bindEntero(con.stmt, 1, asistenteId);
    bindTexto(con.stmt, 2, nombreUsuario, longitudUsuario, 50);
    bindTexto(con.stmt, 3, claveW, longitudClave, 100);
    bindTexto(con.stmt, 4, rolW, longitudRol, 20);

    const string sql =
        "INSERT INTO Usuarios (AsistenteId, NombreUsuario,Clave, Rol) "
        "OUTPUT INSERTED.Id VALUES (?, ?, "
        "CONVERT(NVARCHAR(64), "
        "HASHBYTES('SHA2_256', CAST(? AS NVARCHAR(100))),2))";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar el usuario.");

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID del usuario creado.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID del usuario creado.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool UsuarioDatabase::actualizarUsuario(
    int id,
    const Usuario& u,
    const string& clave,
    const string& rol
) {
    Conexion con(connStr_);

    SQLINTEGER asistenteId = static_cast<SQLINTEGER>(u.asistenteId);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    SQLCHAR activo = u.activo ? 1 : 0;
    wstring nombreUsuario = aWide(u.nombreUsuario);
    wstring claveW = aWide(clave);
    wstring rolW = aWide(rol);

    SQLLEN longitudUsuario = 0;
    SQLLEN longitudClaveCondicion = 0;
    SQLLEN longitudclave = 0;
    SQLLEN longitudRol = 0;

    bindEntero(con.stmt, 1, asistenteId);
    bindTexto(con.stmt, 2, nombreUsuario, longitudUsuario, 50);
    bindBit(con.stmt, 3, activo);
    bindTexto(con.stmt, 4, rolW, longitudRol, 20);
    bindTexto(con.stmt, 5, claveW, longitudClaveCondicion, 100);
    bindTexto(con.stmt, 6, claveW, longitudclave, 100);
    bindEntero(con.stmt, 7, idParam);

    const string sql =
        "UPDATE Usuarios "
        "SET AsistenteId = ?, NombreUsuario = ?, Activo = ?, Rol = ?,"
        "Clave = CASE WHEN LEN(?) = 0 THEN Clave "
        "ELSE CONVERT(NVARCHAR(64), "
        "HASHBYTES('SHA2_256', CAST(? AS NVARCHAR(100))), 2) END "
        "WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar el usuario.");

    return hayFilasAfectadas(con.stmt);
}

// DESACTIVAR (borrado lógico)
bool UsuarioDatabase::desactivarUsuario(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql =
        "UPDATE Usuarios SET Activo = 0 WHERE Id = ? AND Activo = 1";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al desactivar el usuario.");

    return hayFilasAfectadas(con.stmt);
}
