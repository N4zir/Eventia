#include "EventoDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas: 1 = Id, 2 = Nombre, 3 = FechaInicio, 4 = FechaFin
    const string SQL_SELECT_EVENTOS =
        "SELECT Id, Nombre, FechaInicio, FechaFin FROM Eventos";

    Evento leerEvento(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID del evento.");
        }

        Evento e;
        e.id = static_cast<int>(id);
        e.nombre = leerTexto(stmt, 2);
        e.fechaInicio = leerTexto(stmt, 3);
        e.fechaFin = leerTexto(stmt, 4);
        return e;
    }

} // namespace

EventoDatabase::EventoDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Evento> EventoDatabase::listarEventos() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_EVENTOS + " ORDER BY Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar eventos.");

    vector<Evento> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de eventos.");
        lista.push_back(leerEvento(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Evento> EventoDatabase::obtenerEvento(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_EVENTOS + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el evento.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el evento.");

    return leerEvento(con.stmt);
}

// CREAR
int EventoDatabase::crearEvento(const Evento& e) {
    Conexion con(connStr_);

    wstring nombre = aWide(e.nombre);
    wstring fechaInicio = aWide(e.fechaInicio);
    wstring fechaFin = aWide(e.fechaFin);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudInicio = 0;
    SQLLEN longitudFin = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 240);
    bindFecha(con.stmt, 2, fechaInicio, longitudInicio);
    bindFecha(con.stmt, 3, fechaFin, longitudFin);

    const string sql =
        "INSERT INTO Eventos (Nombre, FechaInicio, FechaFin) "
        "OUTPUT INSERTED.Id VALUES (?, ?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar el evento.");

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID del evento creado.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID del evento creado.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool EventoDatabase::actualizarEvento(int id, const Evento& e) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    wstring nombre = aWide(e.nombre);
    wstring fechaInicio = aWide(e.fechaInicio);
    wstring fechaFin = aWide(e.fechaFin);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudInicio = 0;
    SQLLEN longitudFin = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 240);
    bindFecha(con.stmt, 2, fechaInicio, longitudInicio);
    bindFecha(con.stmt, 3, fechaFin, longitudFin);
    bindEntero(con.stmt, 4, idParam);

    const string sql =
        "UPDATE Eventos SET Nombre = ?, FechaInicio = ?, FechaFin = ? "
        "WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar el evento.");

    return hayFilasAfectadas(con.stmt);
}
