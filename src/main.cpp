#include "Syntaxer/Syntaxer.h"
#include "VirtualMachine/vm.h"

#include <filesystem>
#include <iostream>



int main(int argc, char** argv) {
    try {
        std::string fileName = (argc >= 2 ? argv[1] : "test.txt");
       // fileName = resolveInput(fileName);

        Syntaxer syn(fileName);
        syn.Start();

        // syn.PrintPoliz();

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