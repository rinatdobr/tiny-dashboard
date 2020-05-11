#include "i_window.h"

#include <memory>
extern "C" {
#include <X11/Xlib.h>
}

#pragma once

namespace x11 {

using x11Window = ::Window;

class Window : public tdwindows::IWindow
{
public:
    Window(x11Window window);
    ~Window() override;

    x11Window getWindow() const;

private:
    x11Window m_window;
};

} // namespace x11
