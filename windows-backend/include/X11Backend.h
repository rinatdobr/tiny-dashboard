#include <IWindowsBackend.h>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
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

    void setState(const X11BackendState& state);
    bool isReady();
    bool isWorking();
    bool isInvalid();
};

}