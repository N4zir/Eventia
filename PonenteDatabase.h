#pragma once
#include "Ponente.h"
#include <optional>
#include <string>
#include <vector>

class PonenteDatabase {
public:
    explicit PonenteDatabase(std::string connectionString);

    std::vector<Ponente> listarPonentes();
    std::optional<Ponente> obtenerPonente(int id);

    int crearPonente(const Ponente& ponente);
    bool actualizarPonente(int id, const Ponente& ponente);

private:
    std::string connStr_;
};
