#include<iostream>
#include "ApiEvento.h"
using namespace std;

int main()
{
    const string connStr =
        "Driver={ODBC Driver 18 for SQL Server};"
        "Server=.\\SERVER01;"
        "Database=Eventia_DB;"
        "UID=sa;"
        "PWD=4444;"
        "TrustServerCertificate=yes;";

    ApiEvento api(connStr);
    api.iniciar(18080);   // Un solo puerto para todo el taller

    return 0;
}