#pragma once

#include "i_window.h"

#include <memory>
#include <functional>

namespace tdwindows {

using MouseTrackerCb
    = std::function<void(int x, int y)>;

class IWindowsBackend
{
public:
    virtual ~IWindowsBackend() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void setMouseTrackerCallback(MouseTrackerCb callBack) = 0;

protected:
    MouseTrackerCb m_mouseTrackerCb;
};

} // namespace tdwindows
