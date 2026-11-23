#include "EventoDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
using namespace std;

namespace {
const string SQL_SELECT_EVENTOS =
    "SELECT Id, Nombre, FechaInicio, FechaFin FROM Eventos";

Evento leerEvento(SQLHSTMT stmt) {
    SQLINTEGER id = 0;
    SQLGetData(stmt, 1, SQL_C_SLONG, &id, 0, nullptr);
    Evento e;
    e.id = static_cast<int>(id);
    e.nombre = leerTexto(stmt, 2);
    e.fechaInicio = leerTexto(stmt, 3);
    e.fechaFin = leerTexto(stmt, 4);
    return e;
}
}

EventoDatabase::EventoDatabase(string connectionString) : connStr_(connectionString) {}

vector<Evento> EventoDatabase::listarEventos() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_EVENTOS + " ORDER BY Id";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar eventos.");
    vector<Evento> lista;
    while (SQL_SUCCEEDED(SQLFetch(con.stmt))) lista.push_back(leerEvento(con.stmt));
    return lista;
}

optional<Evento> EventoDatabase::obtenerEvento(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = id;
    bindEntero(con.stmt, 1, idParam);
    const string sql = SQL_SELECT_EVENTOS + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el evento.");
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt))) return nullopt;
    return leerEvento(con.stmt);
}

int EventoDatabase::crearEvento(const Evento& e) {
    Conexion con(connStr_);
    wstring nombre = aWide(e.nombre), fechaInicio = aWide(e.fechaInicio), fechaFin = aWide(e.fechaFin);
    SQLLEN l1 = 0, l2 = 0, l3 = 0;
    bindTexto(con.stmt, 1, nombre, l1, 120);
    bindFecha(con.stmt, 2, fechaInicio, l2);
    bindFecha(con.stmt, 3, fechaFin, l3);
    const string sql =
        "INSERT INTO Eventos (Nombre, FechaInicio, FechaFin) OUTPUT INSERTED.Id VALUES (?, ?, ?)";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar el evento.");
    SQLINTEGER id = 0;
    if (SQL_SUCCEEDED(SQLFetch(con.stmt))) SQLGetData(con.stmt, 1, SQL_C_SLONG, &id, 0, nullptr);
    return static_cast<int>(id);
}

bool EventoDatabase::actualizarEvento(int id, const Evento& e) {
    Conexion con(connStr_);
    wstring nombre = aWide(e.nombre), fechaInicio = aWide(e.fechaInicio), fechaFin = aWide(e.fechaFin);
    SQLLEN l1 = 0, l2 = 0, l3 = 0;
    SQLINTEGER idParam = id;
    bindTexto(con.stmt, 1, nombre, l1, 120);
    bindFecha(con.stmt, 2, fechaInicio, l2);
    bindFecha(con.stmt, 3, fechaFin, l3);
    bindEntero(con.stmt, 4, idParam);
    const string sql = "UPDATE Eventos SET Nombre = ?, FechaInicio = ?, FechaFin = ? WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar el evento.");
    return hayFilasAfectadas(con.stmt);
}

// No borrar eventos que tengan charlas asociadas (FK en Eventia_DB).
bool EventoDatabase::eliminarEvento(int id) {
    Conexion con(connStr_);
    SQLINTEGER idDelete = id, idCharla = id;
    bindEntero(con.stmt, 1, idDelete);
    bindEntero(con.stmt, 2, idCharla);
    const string sql =
        "DELETE FROM Eventos WHERE Id = ? "
        "AND NOT EXISTS (SELECT 1 FROM Charlas WHERE EventoId = ?)";
    return ejecutarDelete(con.stmt, sql,
        "Error SQL al eliminar el evento. Revise las relaciones de la base de datos.");
}
