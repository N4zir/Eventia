#include "PonenteDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
using namespace std;

namespace {
const string SQL_SELECT_PONENTES =
    "SELECT Id, Nombre, ISNULL(Institucion, '') FROM Ponentes";

Ponente leerPonente(SQLHSTMT stmt) {
    SQLINTEGER id = 0;
    SQLGetData(stmt, 1, SQL_C_SLONG, &id, 0, nullptr);
    Ponente p;
    p.id = static_cast<int>(id);
    p.nombre = leerTexto(stmt, 2);
    p.institucion = leerTexto(stmt, 3);
    return p;
}
}

PonenteDatabase::PonenteDatabase(string connectionString) : connStr_(connectionString) {}

vector<Ponente> PonenteDatabase::listarPonentes() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_PONENTES + " ORDER BY Id";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar ponentes.");
    vector<Ponente> lista;
    while (SQL_SUCCEEDED(SQLFetch(con.stmt))) lista.push_back(leerPonente(con.stmt));
    return lista;
}

optional<Ponente> PonenteDatabase::obtenerPonente(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = id;
    bindEntero(con.stmt, 1, idParam);
    const string sql = SQL_SELECT_PONENTES + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el ponente.");
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt))) return nullopt;
    return leerPonente(con.stmt);
}

int PonenteDatabase::crearPonente(const Ponente& p) {
    Conexion con(connStr_);
    wstring nombre = aWide(p.nombre), institucion = aWide(p.institucion);
    SQLLEN l1 = 0, l2 = 0;
    bindTexto(con.stmt, 1, nombre, l1, 100);
    bindTexto(con.stmt, 2, institucion, l2, 80);
    const string sql =
        "INSERT INTO Ponentes (Nombre, Institucion) OUTPUT INSERTED.Id VALUES (?, ?)";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar el ponente.");
    SQLINTEGER id = 0;
    if (SQL_SUCCEEDED(SQLFetch(con.stmt))) SQLGetData(con.stmt, 1, SQL_C_SLONG, &id, 0, nullptr);
    return static_cast<int>(id);
}

bool PonenteDatabase::actualizarPonente(int id, const Ponente& p) {
    Conexion con(connStr_);
    wstring nombre = aWide(p.nombre), institucion = aWide(p.institucion);
    SQLLEN l1 = 0, l2 = 0;
    SQLINTEGER idParam = id;
    bindTexto(con.stmt, 1, nombre, l1, 100);
    bindTexto(con.stmt, 2, institucion, l2, 80);
    bindEntero(con.stmt, 3, idParam);
    const string sql = "UPDATE Ponentes SET Nombre = ?, Institucion = ? WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar el ponente.");
    return hayFilasAfectadas(con.stmt);
}

// No borrar ponentes que tengan charlas asociadas (FK en Eventia_DB).
bool PonenteDatabase::eliminarPonente(int id) {
    Conexion con(connStr_);
    SQLINTEGER idDelete = id, idCharla = id;
    bindEntero(con.stmt, 1, idDelete);
    bindEntero(con.stmt, 2, idCharla);
    const string sql =
        "DELETE FROM Ponentes WHERE Id = ? "
        "AND NOT EXISTS (SELECT 1 FROM Charlas WHERE PonenteId = ?)";
    return ejecutarDelete(con.stmt, sql,
        "Error SQL al eliminar el ponente. Revise las relaciones de la base de datos.");
}
