#include <iostream>
#include <memory>
#include <windows-backend/IWindowsBackend.h>
#include <windows-backend/X11Backend.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::info("Welcome to tiny-dashboard");

    std::unique_ptr<TDWindows::IWindowsBackend> windowsBackend(new TDWindows::X11Backend());

    windowsBackend->start();

    std::string command;
    while (true) {
        std::getline(std::cin, command);

        if (! command.compare("q")) {
            windowsBackend->stop();
            break;
        }
    }

    spdlog::info("Bye-bye!");

    return 0;
}
