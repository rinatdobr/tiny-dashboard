#include <windows-backend/IWindowsBackend.h>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
extern "C" {
#include <X11/Xlib.h>
}

#pragma once

namespace TDWindows
{

extern "C" typedef int(*DisplayDeleter)(Display*);

enum class X11BackendState {
    Invalid,
    Ready,
    Working
};

struct WindowInfo {
    WindowInfo();
    WindowInfo(WindowInfo&& info);
    ~WindowInfo();

    Window m_window;
    std::string m_name;
    Window* m_windowChilds;
    unsigned int m_windowNofChilds;
    unsigned int m_level;
    std::vector<WindowInfo> m_childs;
    pid_t m_pid;

    explicit operator std::string() const;
};

class X11Backend : public IWindowsBackend
{
public:
    X11Backend();
    ~X11Backend() override;

    bool start();
    void stop();

private:
    X11BackendState m_state = X11BackendState::Working;
    std::unique_ptr<Display, DisplayDeleter> m_display;

    std::thread m_workingThread;
    bool m_workingThreadFlag = false;
    std::condition_variable m_workingThreadStartNotifier;
    std::mutex m_workingThreadStartMutex;

    void workingThread();
    void getInfo();
    void sendDummyEvent();
    Window* getWindowChilds(const Window &window, unsigned int& resultLen);
    unsigned char* getWindowProperty(const Window &window, const std::string& paramName, unsigned long& resultLen);
    WindowInfo getWindowInfo(const Window &window, const int level = 0);

    void setState(const X11BackendState& state);
    bool isReady();
    bool isWorking();
    bool isInvalid();
};

}