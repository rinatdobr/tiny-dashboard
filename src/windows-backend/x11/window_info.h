#pragma once

#include "window.h"

#include <string>
#include <vector>

namespace x11 {

class WindowInfo
{
public:
    WindowInfo();
    ~WindowInfo();

    void setX11Window(const x11Window x11Window);
    void setName(const std::string &name);
    void setLevel(unsigned int level);
    void setPid(pid_t pid);
    void setSubWindows(std::vector<WindowInfo> subWindows);

    explicit operator std::string() const;

private:
    x11Window m_x11Window = 0;
    std::string m_name = "";
    unsigned int m_level = 0;
    pid_t m_pid = 0;
    std::vector<WindowInfo> m_subWindows;
};

} // namespace x11
