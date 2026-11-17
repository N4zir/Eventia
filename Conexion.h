#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdexcept>
#include <string>
#include "SqlUtf8.h"

#pragma comment(lib, "odbc32.lib")

// Abre la conexión con SQL Server y crea una sentencia lista para usar.
// Cuando el objeto deja de existir, el destructor libera todo solo.
class Conexion {
public:
    SQLHENV  env = nullptr;
    SQLHDBC  dbc = nullptr;
    SQLHSTMT stmt = nullptr;

    explicit Conexion(const std::string& connStr) {
        SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
        SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
        SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

        std::wstring w = aWide(connStr);
        SQLRETURN ret = SQLDriverConnectW(dbc, nullptr,
            (SQLWCHAR*)w.c_str(), SQL_NTS,
            nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);

        if (!SQL_SUCCEEDED(ret)) {
            liberar();
            throw std::runtime_error("No se pudo conectar a SQL Server.");
        }
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    }

    ~Conexion() { liberar(); }

    Conexion(const Conexion&) = delete;
    Conexion& operator=(const Conexion&) = delete;

private:
    void liberar() {
        if (stmt) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        if (dbc) { SQLDisconnect(dbc); SQLFreeHandle(SQL_HANDLE_DBC, dbc); }
        if (env)  SQLFreeHandle(SQL_HANDLE_ENV, env);
        stmt = nullptr;
        dbc = nullptr;
        env = nullptr;
    }
};
