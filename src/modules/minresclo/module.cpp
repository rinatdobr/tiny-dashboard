#include "module.h"
#include <log.h>

namespace minresclo {

std::string Module::application() const
{
    LOG_FUNC_ENTRY();

    return "";
}

void Module::hovered(int x, int y)
{
    LOG_FUNC_ENTRY_MSG_ARGS("x={1}, y={2}", x, y);
}

void Module::clicked(int x, int y)
{
    LOG_FUNC_ENTRY_MSG_ARGS("x={1}, y={2}", x, y);
}

} // namespace minresclo
