#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "crow.h"
#include "crow/middlewares/cors.h"
#include "ApiUsuarios.h"
#include "ApiAsistentes.h"
#include "ApiEventosCrud.h"
#include "ApiPonentes.h"
#include "ApiCharlas.h"
#include "ApiInscripciones.h"
#include "ApiCertificados.h"
//#include "ApiClientes.h"
//#include "ApiVehiculos.h"
//#include "ApiMecanicos.h"
//#include "ApiServicios.h"
//#include "ApiOrdenes.h"
//#include "ApiOrdenServicios.h"
//#include "ApiFacturas.h"
#include <string>

// Servidor único del taller. Agrupa todas las APIs en un solo puerto.
class ApiEvento {
public:
    explicit ApiEvento(const std::string& connStr);

    void iniciar(int puerto);

private:
    void configurarCors();
    void registrarRutas();

    // El orden importa: app_ se crea primero y luego los módulos
    crow::App<crow::CORSHandler> app_;
    ApiUsuarios usuarios_;
    ApiAsistentes asistentes_;
    ApiEventosCrud eventos_;
    ApiPonentes ponentes_;
    ApiCharlas charlas_;
    ApiInscripciones inscripciones_;
    ApiCertificados certificados_;
    //ApiClientes clientes_;
    /* ApiVehiculos vehiculos_;
     ApiMecanicos mecanicos_;
     ApiServicios servicios_;
     ApiOrdenes ordenes_;
     ApiOrdenServicios ordenServicios_;
     ApiFacturas facturas_;*/
};