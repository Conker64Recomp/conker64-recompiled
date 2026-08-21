#pragma once

/*
 * Localizador de assets independiente del directorio de trabajo.
 *
 * Antes cada modulo llevaba su propia lista de prefijos relativos ("", "../",
 * "../../"), y ninguna combinacion funcionaba a la vez para el .exe en
 * recomp/build y para el cwd sugerido en el README. Aqui se resuelve una sola
 * vez buscando `exported_assets` hacia arriba desde el directorio del binario
 * y desde el cwd.
 */

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <SDL.h>

namespace N64 {

class AssetPaths {
public:
    static AssetPaths& getInstance() {
        static AssetPaths instance;
        return instance;
    }

    void init() {
        namespace fs = std::filesystem;
        std::error_code ec;

        std::vector<fs::path> seeds;

        // Directorio del ejecutable: estable sea cual sea el cwd del usuario.
        if (char* base = SDL_GetBasePath()) {
            seeds.emplace_back(base);
            SDL_free(base);
        }
        fs::path cwd = fs::current_path(ec);
        if (!ec) seeds.push_back(cwd);

        for (const auto& seed : seeds) {
            fs::path dir = seed;
            for (int depth = 0; depth <= MAX_WALK_UP; ++depth) {
                fs::path candidate = dir / "exported_assets";
                if (fs::is_directory(candidate, ec)) {
                    assetRoot = candidate;
                    std::cout << "[Assets] Asset root: " << assetRoot.string() << std::endl;
                    return;
                }
                fs::path parent = dir.parent_path();
                if (parent.empty() || parent == dir) break;
                dir = parent;
            }
        }

        std::cerr << "[Assets] WARNING: 'exported_assets' not found near the executable or working directory." << std::endl;
        std::cerr << "[Assets]          ROM models/textures unavailable; procedural fallbacks will be used." << std::endl;
    }

    bool ready() const { return !assetRoot.empty(); }

    // "models/conker_character.obj" -> ruta absoluta existente, o "" si no esta.
    std::string resolve(const std::string& relativeToAssetRoot) const {
        if (assetRoot.empty()) return {};
        std::error_code ec;
        std::filesystem::path p = assetRoot / normalizeSeparators(relativeToAssetRoot);
        return std::filesystem::exists(p, ec) ? p.string() : std::string{};
    }

    // Resuelve una referencia declarada DENTRO de un archivo (p.ej. `map_Kd` de un
    // MTL) contra el directorio de ese archivo, que es la semantica del formato
    // OBJ/MTL. Resolverla contra el cwd es exactamente lo que impedia que
    // cargara ninguna textura de los modelos.
    static std::string resolveRelativeToFile(const std::string& ownerFile,
                                             const std::string& reference) {
        namespace fs = std::filesystem;
        if (reference.empty()) return {};
        std::error_code ec;

        fs::path ref(normalizeSeparators(reference));
        if (ref.is_absolute()) {
            return fs::exists(ref, ec) ? ref.string() : std::string{};
        }

        fs::path owner(ownerFile);
        fs::path combined = owner.parent_path() / ref;

        fs::path resolved = fs::weakly_canonical(combined, ec);
        if (ec) resolved = combined;

        return fs::exists(resolved, ec) ? resolved.string() : std::string{};
    }

private:
    AssetPaths() = default;

    // Los MTL generados por el extractor usan separadores de Windows
    // ("..\textures_rgb\x.png"); normalizarlos mantiene el loader portable.
    static std::string normalizeSeparators(const std::string& in) {
        std::string out = in;
        for (auto& c : out) {
            if (c == '\\') c = '/';
        }
        return out;
    }

    static constexpr int MAX_WALK_UP = 6;
    std::filesystem::path assetRoot;
};

} // namespace N64
