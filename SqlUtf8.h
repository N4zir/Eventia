#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

// ===================== Conversión de texto =====================

// UTF-8 (API y JSON) -> UTF-16 (ODBC Unicode)
inline std::wstring aWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

// UTF-16 (ODBC) -> UTF-8 (API y JSON)
inline std::string aUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

// ===================== Ejecución =====================

// Ejecuta una sentencia SQL con la versión Unicode de ODBC
inline SQLRETURN ejecutar(SQLHSTMT stmt, const std::string& sql) {
    std::wstring w = aWide(sql);
    return SQLExecDirectW(stmt, (SQLWCHAR*)w.c_str(), SQL_NTS);
}

// ===================== Lectura =====================

// Lee una columna de texto y la devuelve en UTF-8. NULL = cadena vacía.
// Importante: leer las columnas en orden ascendente (1, 2, 3...).
inline std::string leerTexto(SQLHSTMT stmt, SQLUSMALLINT columna) {
    SQLWCHAR buffer[1024] = { 0 };
    SQLLEN indicador = 0;
    if (!SQL_SUCCEEDED(SQLGetData(stmt, columna, SQL_C_WCHAR, buffer, sizeof(buffer), &indicador)))
        return "";
    if (indicador == SQL_NULL_DATA)
        return "";
    return aUtf8(std::wstring((const wchar_t*)buffer));
}

// ===================== Parámetros (?) =====================
// El std::wstring debe seguir vivo hasta después de ejecutar().

// Texto obligatorio (una cadena vacía se envía como cadena vacía)
inline void bindTexto(SQLHSTMT stmt, SQLUSMALLINT pos, const std::wstring& w,
    SQLLEN& indicador, SQLULEN tamanio) {
    SQLLEN bytes = static_cast<SQLLEN>(w.size() * sizeof(SQLWCHAR));
    indicador = bytes;
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        tamanio, 0, (SQLPOINTER)w.c_str(), bytes, &indicador);
}

// Fecha ISO (YYYY-MM-DD) -> columna SQL date (SQL Server la convierte sola).
inline void bindFecha(SQLHSTMT stmt, SQLUSMALLINT pos, const std::wstring& w,
    SQLLEN& indicador) {
    SQLLEN bytes = static_cast<SQLLEN>(w.size() * sizeof(SQLWCHAR));
    indicador = bytes;
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        10, 0, (SQLPOINTER)w.c_str(), bytes, &indicador);
}

// Texto opcional: si está vacío se envía NULL
inline void bindTextoOpcional(SQLHSTMT stmt, SQLUSMALLINT pos, const std::wstring& w,
    SQLLEN& indicador, SQLULEN tamanio) {
    SQLLEN bytes = static_cast<SQLLEN>(w.size() * sizeof(SQLWCHAR));
    indicador = w.empty() ? SQL_NULL_DATA : bytes;
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        tamanio, 0, (SQLPOINTER)w.c_str(), bytes, &indicador);
}

// Entero obligatorio
inline void bindEntero(SQLHSTMT stmt, SQLUSMALLINT pos, SQLINTEGER& valor) {
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
        0, 0, &valor, 0, nullptr);
}

// Entero opcional: si el valor es 0 se envía NULL
inline void bindEnteroOpcional(SQLHSTMT stmt, SQLUSMALLINT pos, SQLINTEGER& valor, SQLLEN& indicador) {
    indicador = (valor == 0) ? SQL_NULL_DATA : 0;
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
        0, 0, &valor, 0, &indicador);
}

// Decimal DECIMAL(10,2)
inline void bindDecimal(SQLHSTMT stmt, SQLUSMALLINT pos, SQLDOUBLE& valor) {
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL,
        10, 2, &valor, 0, nullptr);
}

// Booleano (BIT)
inline void bindBit(SQLHSTMT stmt, SQLUSMALLINT pos, SQLCHAR& valor) {
    SQLBindParameter(stmt, pos, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT,
        0, 0, &valor, 0, nullptr);
}

// ===================== Resultado =====================

// Indica si la última sentencia afectó alguna fila
inline bool hayFilasAfectadas(SQLHSTMT stmt) {
    SQLLEN filas = 0;
    SQLRowCount(stmt, &filas);
    return filas > 0;
}
