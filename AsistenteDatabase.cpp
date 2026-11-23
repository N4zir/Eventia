#include "AsistenteDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas:
    // 1 = Id
    // 2 = Nombre
    // 3 = Correo
    const string SQL_SELECT_ASISTENTES =
        "SELECT Id, Nombre, Correo FROM Asistentes";

    Asistente leerAsistente(SQLHSTMT stmt) {
        SQLINTEGER id = 0; 
        SQLGetData(stmt, 1, SQL_C_SLONG, &id, 0, nullptr);
        Asistente a;
        a.id = static_cast<int>(id);
        a.nombre = leerTexto(stmt, 2);
        a.correo = leerTexto(stmt, 3);
        return a;
    }

    // La restriccion UNIQUE de Correo devuelve SQLSTATE 23000 (2601/2627).
    bool esCorreoDuplicado(SQLHSTMT stmt) {
        SQLWCHAR state[10] = { 0 };
        SQLWCHAR message[512] = { 0 };
        SQLINTEGER nativeError = 0;
        SQLSMALLINT length = 0;

        if (!SQL_SUCCEEDED(SQLGetDiagRecW(
            SQL_HANDLE_STMT,
            stmt,
            1,
            state,
            &nativeError,
            message,
            sizeof(message) / sizeof(SQLWCHAR),
            &length))) {
            return false;
        }

        return nativeError == 2601 || nativeError == 2627;
    }

} // namespace

AsistenteDatabase::AsistenteDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Asistente> AsistenteDatabase::listarAsistentes() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_ASISTENTES + " ORDER BY Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar asistentes.");

    vector<Asistente> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de asistentes.");
        lista.push_back(leerAsistente(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Asistente> AsistenteDatabase::obtenerAsistente(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_ASISTENTES + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el asistente.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el asistente.");

    return leerAsistente(con.stmt);
}

// CREAR
int AsistenteDatabase::crearAsistente(const Asistente& a) {
    Conexion con(connStr_);

    wstring nombre = aWide(a.nombre);
    wstring correo = aWide(a.correo);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudCorreo = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 200);
    bindTexto(con.stmt, 2, correo, longitudCorreo, 200);

    const string sql =
        "INSERT INTO Asistentes (Nombre, Correo) "
        "OUTPUT INSERTED.Id VALUES (?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esCorreoDuplicado(con.stmt))
            throw DuplicadoAsistenteException("Ya existe un asistente con ese correo.");
        throw runtime_error("Error al insertar el asistente.");
    }

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID del asistente creado.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID del asistente creado.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool AsistenteDatabase::actualizarAsistente(int id, const Asistente& a) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    wstring nombre = aWide(a.nombre);
    wstring correo = aWide(a.correo);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudCorreo = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 100);
    bindTexto(con.stmt, 2, correo, longitudCorreo, 100);
    bindEntero(con.stmt, 3, idParam);

    const string sql =
        "UPDATE Asistentes SET Nombre = ?, Correo = ? WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esCorreoDuplicado(con.stmt))
            throw DuplicadoAsistenteException("Ya existe un asistente con ese correo.");
        throw runtime_error("Error al actualizar el asistente.");
    }

    return hayFilasAfectadas(con.stmt);
}

bool AsistenteDatabase::eliminarAsistente(int id) {
    Conexion con(connStr_);
    SQLINTEGER idDelete = id, idUsuario = id, idInscripcion = id;
    bindEntero(con.stmt, 1, idDelete);
    bindEntero(con.stmt, 2, idUsuario);
    bindEntero(con.stmt, 3, idInscripcion);
    const string sql =
        "DELETE FROM Asistentes WHERE Id = ? "
        "AND NOT EXISTS (SELECT 1 FROM Usuarios WHERE AsistenteId = ?) "
        "AND NOT EXISTS (SELECT 1 FROM Inscripciones WHERE AsistenteId = ?)";
    return ejecutarDelete(con.stmt, sql,
        "Error SQL al eliminar el asistente. Revise las relaciones de la base de datos.");
}