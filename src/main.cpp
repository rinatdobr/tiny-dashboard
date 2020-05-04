#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::info("Welcome to tiny-dashboard");

    spdlog::info("Bye-bye!");

    return 0;
}
