#pragma once

#include "i_windows_backend.h"
#include "window.h"
#include "window_info.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
extern "C" {
#include <X11/Xlib.h>
}

namespace x11 {

extern "C" typedef int (*DisplayDeleter)(Display *);

class Backend : public tdwindows::IWindowsBackend
{
public:
    Backend();
    ~Backend() override;

    // IWindowsBackend
    bool start() override;
    void stop() override;

    enum class State { Invalid, Ready, Working };

private:
    State m_state = State::Invalid;
    std::unique_ptr<Display, DisplayDeleter> m_display;
    std::vector<std::shared_ptr<Window>> m_createdWindows;

    std::thread m_workingThread;
    bool m_workingThreadFlag = false;
    std::condition_variable m_workingThreadStartNotifier;
    std::mutex m_workingThreadStartMutex;

    std::vector<std::string> m_State_str = {"Invalid", "Ready", "Working"};

    void workingThread();
    void getDisplayInfo();
    void sendDummyEvent();
    std::vector<x11Window> getWindowChilds(const x11Window &window);
    std::vector<char> getWindowProperty(const x11Window &window,
                                        const std::string &paramName);
    WindowInfo getWindowInfo(const x11Window &window, const int level = 0);
    void setState(const State &state);
    bool isReady();
    bool isWorking();
    bool isInvalid();
};

} // namespace x11
