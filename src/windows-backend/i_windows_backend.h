#pragma once

#include "i_window.h"

#include <memory>

namespace tdwindows {

class IWindowsBackend
{
public:
    virtual ~IWindowsBackend() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
};

} // namespace tdwindows
