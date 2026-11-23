#include "CharlaDatabase.h"
#include "Conexion.h"
#include "SqlUtf8.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    // Columnas: 1 = Id, 2 = EventoId, 3 = Evento.Nombre, 4 = Sala,
    // 5 = PonenteId, 6 = Ponente.Nombre, 7 = Ponente.Institucion,
    // 8 = Titulo, 9 = HoraInicio, 10 = Cupo
    const string SQL_SELECT_CHARLAS =
        "SELECT c.Id, c.EventoId, e.Nombre, c.Sala, c.PonenteId, "
        "p.Nombre, p.Institucion, c.Titulo, c.HoraInicio, c.Cupo "
        "FROM Charlas AS c "
        "INNER JOIN Eventos AS e ON e.Id = c.EventoId "
        "INNER JOIN Ponentes AS p ON p.Id = c.PonenteId";

    Charla leerCharla(SQLHSTMT stmt) {
        SQLINTEGER id = 0;
        SQLINTEGER eventoId = 0;
        SQLINTEGER ponenteId = 0;
        SQLINTEGER cupo = 0;

        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
            throw runtime_error("No se pudo leer el ID de la charla.");
        }
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 2, SQL_C_SLONG, &eventoId, sizeof(eventoId), nullptr))) {
            throw runtime_error("No se pudo leer el ID del evento.");
        }
        string eventoNombre = leerTexto(stmt, 3);
        string sala = leerTexto(stmt, 4);
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 5, SQL_C_SLONG, &ponenteId, sizeof(ponenteId), nullptr))) {
            throw runtime_error("No se pudo leer el ID del ponente.");
        }
        string ponenteNombre = leerTexto(stmt, 6);
        string ponenteInstitucion = leerTexto(stmt, 7);
        string titulo = leerTexto(stmt, 8);
        string horaInicio = leerTexto(stmt, 9);
        if (!SQL_SUCCEEDED(SQLGetData(
            stmt, 10, SQL_C_SLONG, &cupo, sizeof(cupo), nullptr))) {
            throw runtime_error("No se pudo leer el cupo de la charla.");
        }

        Charla c;
        c.id = static_cast<int>(id);
        c.eventoId = static_cast<int>(eventoId);
        c.eventoNombre = eventoNombre;
        c.sala = sala;
        c.ponenteId = static_cast<int>(ponenteId);
        c.ponenteNombre = ponenteNombre;
        c.ponenteInstitucion = ponenteInstitucion;
        c.titulo = titulo;
        c.horaInicio = horaInicio;
        c.cupo = static_cast<int>(cupo);
        return c;
    }

} // namespace

CharlaDatabase::CharlaDatabase(string connectionString)
    : connStr_(std::move(connectionString)) {
}

// LISTAR
vector<Charla> CharlaDatabase::listarCharlas() {
    Conexion con(connStr_);
    const string sql = SQL_SELECT_CHARLAS + " ORDER BY c.Id";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al listar charlas.");

    vector<Charla> lista;
    while (true) {
        SQLRETURN resultado = SQLFetch(con.stmt);
        if (resultado == SQL_NO_DATA)
            break;
        if (!SQL_SUCCEEDED(resultado))
            throw runtime_error("Error al recuperar la lista de charlas.");
        lista.push_back(leerCharla(con.stmt));
    }
    return lista;
}

// OBTENER POR ID
optional<Charla> CharlaDatabase::obtenerCharla(int id) {
    Conexion con(connStr_);
    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    bindEntero(con.stmt, 1, idParam);

    const string sql = SQL_SELECT_CHARLAS + " WHERE c.Id = ?";
    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al consultar la charla.");

    SQLRETURN resultado = SQLFetch(con.stmt);
    if (resultado == SQL_NO_DATA)
        return nullopt;
    if (!SQL_SUCCEEDED(resultado))
        throw runtime_error("Error al recuperar la charla.");

    return leerCharla(con.stmt);
}

// EXISTENCIA DE FORANEAS
bool CharlaDatabase::existeEvento(int eventoId) {
    return existePorId(connStr_, "SELECT 1 FROM Eventos WHERE Id = ?", eventoId);
}

bool CharlaDatabase::existePonente(int ponenteId) {
    return existePorId(connStr_, "SELECT 1 FROM Ponentes WHERE Id = ?", ponenteId);
}

// CREAR
int CharlaDatabase::crearCharla(const Charla& c) {
    Conexion con(connStr_);

    SQLINTEGER eventoId = static_cast<SQLINTEGER>(c.eventoId);
    SQLINTEGER ponenteId = static_cast<SQLINTEGER>(c.ponenteId);
    SQLINTEGER cupo = static_cast<SQLINTEGER>(c.cupo);
    wstring sala = aWide(c.sala);
    wstring titulo = aWide(c.titulo);
    wstring horaInicio = aWide(c.horaInicio);
    SQLLEN longitudSala = 0;
    SQLLEN longitudTitulo = 0;
    SQLLEN longitudHora = 0;

    bindEntero(con.stmt, 1, eventoId);
    bindTexto(con.stmt, 2, sala, longitudSala, 40);
    bindEntero(con.stmt, 3, ponenteId);
    bindTexto(con.stmt, 4, titulo, longitudTitulo, 300);
    bindTexto(con.stmt, 5, horaInicio, longitudHora, 10);
    bindEntero(con.stmt, 6, cupo);

    const string sql =
        "INSERT INTO Charlas (EventoId, Sala, PonenteId, Titulo, HoraInicio, Cupo) "
        "OUTPUT INSERTED.Id VALUES (?, ?, ?, ?, ?, ?)";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al insertar la charla.");

    SQLINTEGER id = 0;
    if (!SQL_SUCCEEDED(SQLFetch(con.stmt)))
        throw runtime_error("No se pudo recuperar el ID de la charla creada.");

    if (!SQL_SUCCEEDED(SQLGetData(
        con.stmt, 1, SQL_C_SLONG, &id, sizeof(id), nullptr))) {
        throw runtime_error("No se pudo leer el ID de la charla creada.");
    }

    return static_cast<int>(id);
}

// ACTUALIZAR
bool CharlaDatabase::actualizarCharla(int id, const Charla& c) {
    Conexion con(connStr_);

    SQLINTEGER idParam = static_cast<SQLINTEGER>(id);
    SQLINTEGER eventoId = static_cast<SQLINTEGER>(c.eventoId);
    SQLINTEGER ponenteId = static_cast<SQLINTEGER>(c.ponenteId);
    SQLINTEGER cupo = static_cast<SQLINTEGER>(c.cupo);
    wstring sala = aWide(c.sala);
    wstring titulo = aWide(c.titulo);
    wstring horaInicio = aWide(c.horaInicio);
    SQLLEN longitudSala = 0;
    SQLLEN longitudTitulo = 0;
    SQLLEN longitudHora = 0;

    bindEntero(con.stmt, 1, eventoId);
    bindTexto(con.stmt, 2, sala, longitudSala, 40);
    bindEntero(con.stmt, 3, ponenteId);
    bindTexto(con.stmt, 4, titulo, longitudTitulo, 300);
    bindTexto(con.stmt, 5, horaInicio, longitudHora, 10);
    bindEntero(con.stmt, 6, cupo);
    bindEntero(con.stmt, 7, idParam);

    const string sql =
        "UPDATE Charlas SET EventoId = ?, Sala = ?, PonenteId = ?, "
        "Titulo = ?, HoraInicio = ?, Cupo = ? WHERE Id = ?";

    if (!SQL_SUCCEEDED(ejecutar(con.stmt, sql)))
        throw runtime_error("Error al actualizar la charla.");

    return hayFilasAfectadas(con.stmt);
}

// ELIMINAR
// No borrar charlas que tengan inscripciones (FK en Eventia_DB).
bool CharlaDatabase::eliminarCharla(int id) {
    Conexion con(connStr_);
    SQLINTEGER idDelete = id, idInscripcion = id;
    bindEntero(con.stmt, 1, idDelete);
    bindEntero(con.stmt, 2, idInscripcion);
    const string sql =
        "DELETE FROM Charlas WHERE Id = ? "
        "AND NOT EXISTS (SELECT 1 FROM Inscripciones WHERE CharlaId = ?)";
    return ejecutarDelete(con.stmt, sql,
        "Error SQL al eliminar la charla. Revise las relaciones de la base de datos.");
}
