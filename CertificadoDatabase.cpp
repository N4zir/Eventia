#include "CertificadoDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas: 1 = Id, 2 = InscripcionId, 3 = Asistente.Nombre,
    // 4 = Charla.Titulo, 5 = Codigo, 6 = FechaEmision
    const string SQL_SELECT_CERTIFICADOS =
        "SELECT ce.Id, ce.InscripcionId, a.Nombre, ch.Titulo, "
        "ce.Codigo, ce.FechaEmision "
        "FROM Certificados AS ce "
        "INNER JOIN Inscripciones AS i ON i.Id = ce.InscripcionId "
        "INNER JOIN Asistentes AS a ON a.Id = i.AsistenteId "
        "INNER JOIN Charlas AS ch ON ch.Id = i.CharlaId";

    Certificado leerCertificado(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        SQLINTEGER inscripcionId = 0;

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID del certificado.");
        }
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 2, SQL_C_SLONG, &inscripcionId, sizeof(inscripcionId), nullptr))) {
            throw runtime_error("No se pudo leer el ID de la inscripcion.");
        }
        string asistenteNombre = leerTexto(stmt, 3);
        string charlaTitulo = leerTexto(stmt, 4);

        Certificado c;
        c.id = static_cast<int>(id);
        c.inscripcionId = static_cast<int>(inscripcionId);
        c.asistenteNombre = asistenteNombre;
        c.charlaTitulo = charlaTitulo;
        c.codigo = leerTexto(stmt, 5);
        c.fechaEmision = leerTexto(stmt, 6);
        return c;
    }

    // Las UNIQUE de InscripcionId y Codigo devuelven 2601/2627.
    bool esCertificadoDuplicado(SQLHSTMT stmt) {
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

CertificadoDatabase::CertificadoDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Certificado> CertificadoDatabase::listarCertificados() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_CERTIFICADOS + " ORDER BY ce.Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar certificados.");

    vector<Certificado> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de certificados.");
        lista.push_back(leerCertificado(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Certificado> CertificadoDatabase::obtenerCertificado(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_CERTIFICADOS + " WHERE ce.Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar el certificado.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar el certificado.");

    return leerCertificado(con.stmt);
}

// EXISTENCIA DE FORANEA
bool CertificadoDatabase::existeInscripcion(int inscripcionId) {
    return existePorId(connStr_, "SELECT 1 FROM Inscripciones WHERE Id = ?", inscripcionId);
}

// CREAR
int CertificadoDatabase::crearCertificado(const Certificado& c) {
    Conexion con(connStr_);

    SQLINTEGER inscripcionId = static_cast<SQLINTEGER>(c.inscripcionId);
    wstring codigo = aWide(c.codigo);
    wstring fechaEmision = aWide(c.fechaEmision);
    SQLLEN longitudCodigo = 0;
    SQLLEN longitudEmision = 0;

    bindEntero(con.stmt, 1, inscripcionId);
    bindTexto(con.stmt, 2, codigo, longitudCodigo, 40);
    bindFecha(con.stmt, 3, fechaEmision, longitudEmision);

    const string sql =
        "INSERT INTO Certificados (InscripcionId, Codigo, FechaEmision) "
        "OUTPUT INSERTED.Id VALUES (?, ?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esCertificadoDuplicado(con.stmt))
            throw DuplicadoCertificadoException("Ya existe un certificado con esa inscripcion o codigo.");
        throw runtime_error("Error al insertar el certificado.");
    }

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID del certificado creado.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID del certificado creado.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool CertificadoDatabase::actualizarCertificado(int id, const Certificado& c) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    SQLINTEGER inscripcionId = static_cast<SQLINTEGER>(c.inscripcionId);
    wstring codigo = aWide(c.codigo);
    wstring fechaEmision = aWide(c.fechaEmision);
    SQLLEN longitudCodigo = 0;
    SQLLEN longitudEmision = 0;

    bindEntero(con.stmt, 1, inscripcionId);
    bindTexto(con.stmt, 2, codigo, longitudCodigo, 40);
    bindFecha(con.stmt, 3, fechaEmision, longitudEmision);
    bindEntero(con.stmt, 4, idParam);

    const string sql =
        "UPDATE Certificados SET InscripcionId = ?, Codigo = ?, "
        "FechaEmision = ? WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql))) {
        if (esCertificadoDuplicado(con.stmt))
            throw DuplicadoCertificadoException("Ya existe un certificado con esa inscripcion o codigo.");
        throw runtime_error("Error al actualizar el certificado.");
    }

    return hayFilasAfectadas(con.stmt);
}

// ELIMINAR
bool CertificadoDatabase::eliminarCertificado(int id) {
    Conexion con(connStr_);
    SQLINTEGER idDelete = id;
    bindEntero(con.stmt, 1, idDelete);
    const string sql = "DELETE FROM Certificados WHERE Id = ?";
    return ejecutarDelete(con.stmt, sql,
        "Error SQL al eliminar el certificado. Revise las relaciones de la base de datos.");
}
