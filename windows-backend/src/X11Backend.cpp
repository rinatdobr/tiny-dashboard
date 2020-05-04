#include <X11Backend.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace TDWindows
{

X11Backend::X11Backend() :
    m_display(nullptr, XCloseDisplay)
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    setState(X11BackendState::Ready);
}

X11Backend::~X11Backend()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    stop();
}

bool X11Backend::start()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    if (! isReady()) {
        spdlog::error("X11 backend is not ready");
        return false;
    }

    spdlog::info("X11 backend is starting...");
    
    std::unique_lock<std::mutex> _ul(m_workingThreadStartMutex);
    m_workingThread = std::thread(&X11Backend::workingThread, this);
    m_workingThreadStartNotifier.wait(_ul, [this]{ return isInvalid() || isWorking(); });

    if (isWorking()) {
        spdlog::info("X11 backend started");
        return true;
    }
    else {
        spdlog::error("X11 backend didn't start");
        return false;
    }
}

void X11Backend::stop()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    if (isWorking()) {
        m_workingThreadFlag = false;
        sendDummyEvent();
        m_workingThread.join();
    }

    setState(X11BackendState::Ready);

    spdlog::info("X11 backend stopped");
}

void X11Backend::workingThread()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    spdlog::info("X11 backend working thread starting...");

    {
        std::lock_guard<std::mutex> _ul(m_workingThreadStartMutex);
        m_display.reset(XOpenDisplay(NULL));
        if (! m_display.get()) {
            spdlog::error("Can't open display");
            setState(X11BackendState::Invalid);
            return;
        }
        setState(X11BackendState::Working);
        getInfo();
        m_workingThreadFlag = true;
    }
    m_workingThreadStartNotifier.notify_one();

    spdlog::info("Started X11 working thread");

    Window rootWindow = XDefaultRootWindow(m_display.get());
    unsigned int eventMask = ButtonPressMask | PointerMotionMask;
    XEvent event;
    
    if (XGrabPointer(m_display.get(), rootWindow, False, eventMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
        spdlog::error("Can't grab pointer");
        setState(X11BackendState::Invalid);
        m_display.reset();
        return;
    }

    XSelectInput(m_display.get(), rootWindow, eventMask);
    while(m_workingThreadFlag) {
        XNextEvent(m_display.get(), &event);
        switch (event.type) {
            case ButtonPress: {
                    XButtonEvent* buttonEvent = reinterpret_cast<XButtonEvent*>(&event);
                    spdlog::trace("Button event: {0}", buttonEvent->button);
                }
                break;
            case MotionNotify: {
                    XMotionEvent* motionEvent = reinterpret_cast<XMotionEvent*>(&event);
                    spdlog::trace("Motion event: x: {0}, y: {1}", motionEvent->x, motionEvent->y);
                }
                break;
        }
    }

    XUngrabPointer(m_display.get(), CurrentTime);
    m_display.reset();
    
    spdlog::info("X11 backend working thread finished");
}

void X11Backend::getInfo()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);
    
    int nofScreens = XScreenCount(m_display.get());

    spdlog::debug("Display info: ");
    spdlog::debug("\tNumber of screens: {0}", nofScreens);
    for (int i = 0; i < nofScreens; i++) {
        spdlog::debug("\tScreen info [{0}]", i);
        spdlog::debug("\t\twidth: {0}", XDisplayWidth(m_display.get(), i));
        spdlog::debug("\t\theight: {0}", XDisplayHeight(m_display.get(), i));
    }
}

void X11Backend::sendDummyEvent()
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    if (! isWorking()) {
        return;
    }

    XEvent event = {};
    event.type = MotionNotify;
    event.xmotion.x = 0;
    event.xmotion.y = 0;
    event.xmotion.x_root = 0;
    event.xmotion.y_root = 0;
    event.xmotion.time = CurrentTime;
    event.xmotion.same_screen = True;

    if (! XSendEvent(m_display.get(), XDefaultRootWindow(m_display.get()), True, PointerMotionMask, &event)) {
        spdlog::error("Can't send dummy event");
        return;
    }

    XFlush(m_display.get());
}

void X11Backend::setState(const X11BackendState& state)
{
    static std::mutex _stateMutex;

    std::lock_guard<std::mutex> _stateLg(_stateMutex);
    m_state = state;
}

bool X11Backend::isInvalid()
{
    return m_state == X11BackendState::Invalid;
}

bool X11Backend::isReady()
{
    return m_state == X11BackendState::Ready;
}

bool X11Backend::isWorking()
{
    return m_state == X11BackendState::Working;
}

}