#include "InscripcionDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas: 1 = Id, 2 = AsistenteId, 3 = CharlaId, 4 = Fecha, 5 = Asistio
    const string SQL_SELECT_INSCRIPCIONES =
        "SELECT Id, AsistenteId, CharlaId, Fecha, Asistio FROM Inscripciones";

    Inscripcion leerInscripcion(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        SQLINTEGER asistenteId = 0;
        SQLINTEGER charlaId = 0;
        SQLCHAR asistio = 0;

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID de la inscripcion.");
        }
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 2, SQL_C_SLONG, &asistenteId, sizeof(asistenteId), nullptr))) {
            throw runtime_error("No se pudo leer el ID del asistente.");
        }
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 3, SQL_C_SLONG, &charlaId, sizeof(charlaId), nullptr))) {
            throw runtime_error("No se pudo leer el ID de la charla.");
        }

        Inscripcion i;
        i.id = static_cast<int>(id);
        i.asistenteId = static_cast<int>(asistenteId);
        i.charlaId = static_cast<int>(charlaId);
        i.fecha = leerTexto(stmt, 4);

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 5, SQL_C_BIT, &asistio, sizeof(asistio), nullptr))) {
            throw runtime_error("No se pudo leer el estado de asistencia.");
        }
        i.asistio = (asistio != 0);
        return i;
    }

    // Consulta puntual de existencia: no hace JOIN ni trae datos de otra tabla.
    bool existePorId(const string& connStr, const string& sql, int id) {
        Conexion con(connStr);
        SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
        bindEntero(con.stmt, 1, idParam);

        if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
            throw runtime_error("Error al consultar la existencia del registro.");

        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            return false;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al leer la consulta de existencia.");
        return true;
    }

    // La UNIQUE compuesta de AsistenteId + CharlaId devuelve 2601/2627.
    bool esInscripcionDuplicada(SQLHSTMT stmt) {
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

InscripcionDatabase::InscripcionDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Inscripcion> InscripcionDatabase::listarInscripciones() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_INSCRIPCIONES + " ORDER BY Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar inscripciones.");

    vector<Inscripcion> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de inscripciones.");
        lista.push_back(leerInscripcion(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Inscripcion> InscripcionDatabase::obtenerInscripcion(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_INSCRIPCIONES + " WHERE Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar la inscripcion.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar la inscripcion.");

    return leerInscripcion(con.stmt);
}

// EXISTENCIA DE FORANEAS
bool InscripcionDatabase::existeAsistente(int asistenteId) {
    return existePorId(connStr_, "SELECT 1 FROM Asistentes WHERE Id = ?", asistenteId);
}

bool InscripcionDatabase::existeCharla(int charlaId) {
    return existePorId(connStr_, "SELECT 1 FROM Charlas WHERE Id = ?", charlaId);
}

// CREAR
int InscripcionDatabase::crearInscripcion(const Inscripcion& i) {
    Conexion con(connStr_);

    SQLINTEGER asistenteId = static_cast<SQLINTEGER>(i.asistenteId);
    SQLINTEGER charlaId = static_cast<SQLINTEGER>(i.charlaId);
    SQLCHAR asistio = i.asistio ? 1 : 0;
    wstring fecha = aWide(i.fecha);
    SQLLEN longitudFecha = 0;

    bindEntero(con.stmt, 1, asistenteId);
    bindEntero(con.stmt, 2, charlaId);
    bindFecha(con.stmt, 3, fecha, longitudFecha);
    bindBit(con.stmt, 4, asistio);

    const string sql =
        "INSERT INTO Inscripciones (AsistenteId, CharlaId, Fecha, Asistio) "
        "OUTPUT INSERTED.Id VALUES (?, ?, ?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esInscripcionDuplicada(con.stmt))
            throw DuplicadoInscripcionException("El asistente ya esta inscrito en esa charla.");
        throw runtime_error("Error al insertar la inscripcion.");
    }

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID de la inscripcion creada.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID de la inscripcion creada.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool InscripcionDatabase::actualizarInscripcion(int id, const Inscripcion& i) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    SQLINTEGER asistenteId = static_cast<SQLINTEGER>(i.asistenteId);
    SQLINTEGER charlaId = static_cast<SQLINTEGER>(i.charlaId);
    SQLCHAR asistio = i.asistio ? 1 : 0;
    wstring fecha = aWide(i.fecha);
    SQLLEN longitudFecha = 0;

    bindEntero(con.stmt, 1, asistenteId);
    bindEntero(con.stmt, 2, charlaId);
    bindFecha(con.stmt, 3, fecha, longitudFecha);
    bindBit(con.stmt, 4, asistio);
    bindEntero(con.stmt, 5, idParam);

    const string sql =
        "UPDATE Inscripciones SET AsistenteId = ?, CharlaId = ?, "
        "Fecha = ?, Asistio = ? WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esInscripcionDuplicada(con.stmt))
            throw DuplicadoInscripcionException("El asistente ya esta inscrito en esa charla.");
        throw runtime_error("Error al actualizar la inscripcion.");
    }

    return hayFilasAfectadas(con.stmt);
}
