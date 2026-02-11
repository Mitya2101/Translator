//#define _GLIBCXX_DEBUG
#include "Syntaxer.h"
#include "vm.h"

#include <filesystem>
#include <iostream>

static std::string resolveInput(const std::string& name) {
    namespace fs = std::filesystem;

    fs::path p(name);
    if (p.is_absolute() && fs::exists(p)) return p.string();

    fs::path cur = fs::current_path();

    // 1) текущая папка запуска
    fs::path c1 = cur / p;
    if (fs::exists(c1)) return c1.string();

    // 2) на уровень выше (обычно это корень проекта если запускаешь из cmake-build-debug)
    fs::path c2 = cur / ".." / p;
    if (fs::exists(c2)) return c2.string();

    // 3) ещё уровень выше (на всякий)
    fs::path c3 = cur / ".." / ".." / p;
    if (fs::exists(c3)) return c3.string();

    // fallback — пусть Lexer сам попробует
    return name;
}

int main(int argc, char** argv) {
    try {
        std::string fileName = (argc >= 2 ? argv[1] : "test.txt");
        fileName = resolveInput(fileName);

        Syntaxer syn(fileName);
        syn.Start();

        PolizVm vm(syn.poliz, syn.func);
        vm.runAuto(); // main() если есть, иначе выполнить POLIZ с 0

        return 0;
    }
    catch (const std::string& e) {
        std::cerr << "Error: " << e << "\n";
    }
    catch (const char* e) {
        std::cerr << "Error: " << e << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "std::exception: " << e.what() << "\n";
    }
    return 1;
}
