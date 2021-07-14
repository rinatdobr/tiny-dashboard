#pragma once

#include <string>

namespace tdmodules {

class IModule
{
public:
    std::string application();
    void hovered(int x, int y);
    void clicked(int x, int y);
};

} // namespace tdplugins
