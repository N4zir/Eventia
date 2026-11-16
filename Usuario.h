#pragma once
#include <string>

// Representa un registro de la tabla Usuarios.
// Los datos del cliente se obtienen mediante JOIN con Clientes.
// La clave nunca se devuelve en el modelo ni en las respuestas JSON.
struct Usuario {
    int id = 0;
    int asistenteId = 0;
    std::string asistenteNombre;
    std::string nombreUsuario;
    std::string clave;
    std::string rol;
    bool activo = true;   
};
