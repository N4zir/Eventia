#include "PonenteDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas: 1 = Id, 2 = Nombre, 3 = Institucion
    const string SQL_SELECT_PONENTES =
        "SELECT Id, Nombre, Institucion FROM Ponentes";

    Ponente leerPonente(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID del ponente.");
        }

        Ponente p;
        p.id = static_cast<int>(id);
        p.nombre = leerTexto(stmt, 2);
        p.institucion = leerTexto(stmt, 3);
        return p;
    }

} // namespace

PonenteDatabase::PonenteDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Ponente> PonenteDatabase::listarPonentes() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_PONENTES + " ORDER BY Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar ponentes.");

    vector<Ponente> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de ponentes.");
        lista.push_back(leerPonente(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Ponente> PonenteDatabase::obtenerPonente(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_PONENTES + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el ponente.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el ponente.");

    return leerPonente(con.stmt);
}

// CREAR
int PonenteDatabase::crearPonente(const Ponente& p) {
    Conexion con(connStr_);

    wstring nombre = aWide(p.nombre);
    wstring institucion = aWide(p.institucion);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudInstitucion = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 200);
    bindTexto(con.stmt, 2, institucion, longitudInstitucion, 160);

    const string sql =
        "INSERT INTO Ponentes (Nombre, Institucion) "
        "OUTPUT INSERTED.Id VALUES (?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar el ponente.");

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID del ponente creado.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID del ponente creado.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool PonenteDatabase::actualizarPonente(int id, const Ponente& p) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    wstring nombre = aWide(p.nombre);
    wstring institucion = aWide(p.institucion);
    SQLLEN longitudNombre = 0;
    SQLLEN longitudInstitucion = 0;

    bindTexto(con.stmt, 1, nombre, longitudNombre, 200);
    bindTexto(con.stmt, 2, institucion, longitudInstitucion, 160);
    bindEntero(con.stmt, 3, idParam);

    const string sql =
        "UPDATE Ponentes SET Nombre = ?, Institucion = ? WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar el ponente.");

    return hayFilasAfectadas(con.stmt);
}
