#pragma once
#include "Charla.h"
#include <optional>
#include <string>
#include <vector>

class CharlaDatabase {
public:
    explicit CharlaDatabase(std::string connectionString);

    std::vector<Charla> listarCharlas();
    std::optional<Charla> obtenerCharla(int id);

    bool existeEvento(int eventoId);
    bool existePonente(int ponenteId);

    int crearCharla(const Charla& charla);
    bool actualizarCharla(int id, const Charla& charla);

private:
    std::string connStr_;
};
