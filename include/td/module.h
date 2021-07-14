#pragma once

#include <string>

namespace tdmodules {

class Module
{
public:
    virtual ~Module() = default;

    virtual std::string application() const = 0;
    virtual void hovered(int x, int y) = 0;
    virtual void clicked(int x, int y) = 0;
};

} // namespace tdplugins
