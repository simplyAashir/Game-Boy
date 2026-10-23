#include "Frontend.hpp"
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
int main(int argc, char **argv) {
    try {
        // Double-clicking and launching from VS Code resolve the same asset/data paths.
#ifdef _WIN32
        char path[32768]{};
        DWORD size = GetModuleFileNameA(nullptr, path, sizeof(path));
        if (size > 0 && size < sizeof(path)) {
            auto folder = std::filesystem::path(path).parent_path();
            if (std::filesystem::exists(folder / "assets"))
                std::filesystem::current_path(folder);
            else if (std::filesystem::exists(folder.parent_path() / "assets"))
                std::filesystem::current_path(folder.parent_path());
        }
#endif
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        pocket::GameBoy app;
        return app.run(argc == 3 && std::string(argv[1]) == "--preview" ? argv[2] : "");
    } catch (const std::exception &e) {
        std::cerr << "Fantasy Quest could not start: " << e.what() << '\n';
        return 1;
    }
}
