#include "window.h"
#include "log.h"

namespace x11 {

Window::Window(x11Window window)
    : m_window(window)
{
    LOG_FUNC_ENTRY();
}

Window::~Window()
{
    LOG_FUNC_ENTRY();
}

x11Window Window::getWindow() const
{
    LOG_FUNC_ENTRY();

    return m_window;
}

} // namespace x11
