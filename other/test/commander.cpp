#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream pipe("/tmp/my_pipe");

    if (!pipe.is_open()) {
        std::cerr << "Failed to open pipe!" << std::endl;
        return 1;
    }

    std::string command;
    while (true) {
        std::getline(pipe, command);  // Read from the pipe
        if (command == "execute_task") {
            std::cout << "Task executed!" << std::endl;
        }
    }

    pipe.close();
    return 0;
}
