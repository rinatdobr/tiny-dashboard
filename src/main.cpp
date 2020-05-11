#include "ui/centre.h"
#include "windows-backend/i_window.h"
#include "windows-backend/i_windows_backend.h"
#include "windows-backend/x11/backend.h"
#include <iostream>
#include <memory>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::info("Welcome to tiny-dashboard");

    std::unique_ptr<tdwindows::IWindowsBackend> windowsBackend(new x11::Backend());
    windowsBackend->start();

    std::string command;
    while (true) {
        std::getline(std::cin, command);

        if (!command.compare("q")) {
            windowsBackend->stop();
            break;
        }
    }

    spdlog::info("Bye-bye!");

    return 0;
}
