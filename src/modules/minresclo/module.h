#pragma once

#include <td/module.h>

namespace minresclo {

class Module : public tdmodules::Module
{
public:
    virtual ~Module() = default;

    std::string application() const override;
    void hovered(int x, int y) override;
    void clicked(int x, int y) override;
};

} // namespace tdplugins
