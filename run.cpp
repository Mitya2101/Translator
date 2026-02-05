#include "Syntaxer.h"
#include "vm.h"

#include <iostream>

int main() {
    try {
        Syntaxer s("test.txt");
        s.Start();

        VM vm(s.poliz, s.func);
        // vm.setTrace(true); // uncomment for VM trace
        vm.run();

    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        return 1;
    } catch (const std::string& e) {
        std::cerr << "Error: " << e << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error\n";
        return 1;
    }
    return 0;
}
