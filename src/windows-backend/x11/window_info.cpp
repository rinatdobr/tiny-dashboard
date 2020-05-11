#include "window_info.h"
#include "log.h"

#include <sstream>
extern "C" {
#include <X11/Xlib.h>
}

namespace x11 {

WindowInfo::WindowInfo()
    : m_name("undefined")
    , m_level(0)
{
    LOG_FUNC_ENTRY();
}

WindowInfo::~WindowInfo()
{
    // LOG_FUNC_ENTRY();
}

void WindowInfo::setX11Window(const x11Window x11Window)
{
    m_x11Window = x11Window;
}

void WindowInfo::setName(const std::string &name)
{
    m_name = name;
}

void WindowInfo::setLevel(unsigned int level)
{
    m_level = level;
}

void WindowInfo::setPid(pid_t pid)
{
    m_pid = pid;
}

void WindowInfo::setSubWindows(std::vector<WindowInfo> subWindows)
{
    m_subWindows = subWindows;
}

WindowInfo::operator std::string() const
{
    // LOG_FUNC_ENTRY();

    std::ostringstream stream;
    stream << "\n";
    for (int i = 0; i < m_level; i++)
        stream << "\t";
    stream << "\t[id=0x" << std::hex << m_x11Window << std::dec;
    stream << ", name=\"" << m_name << "\"";
    stream << ", pid: " << m_pid;
    stream << ", nofSubWindows=" << m_subWindows.size() << "]";
    for (const auto &subWindow : m_subWindows) {
        stream << std::string(subWindow);
    }

    return stream.str();
}

} // namespace x11
