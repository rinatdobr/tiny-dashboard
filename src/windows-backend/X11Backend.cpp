#include <windows-backend/X11Backend.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>

namespace TDWindows
{

WindowInfo::WindowInfo()
 :
        m_name("undefined"),
        m_windowChilds(nullptr),
        m_level(0)
{ }

WindowInfo::WindowInfo(WindowInfo&& info) : 
    m_window(info.m_window),
    m_name(std::move(info.m_name)),
    m_windowChilds(info.m_windowChilds),
    m_windowNofChilds(info.m_windowNofChilds),
    m_level(info.m_level),
    m_childs(std::move(info.m_childs)),
    m_pid(info.m_pid)
{
    info.m_windowChilds = nullptr;
}


WindowInfo::~WindowInfo()
{
    XFree(m_windowChilds);
}

WindowInfo::operator std::string() const
{
    std::ostringstream stream;
    stream << "\n";
    for (int i = 0; i < m_level; i++) stream << "\t";
    stream << "WindowInfo(0x" << std::hex << m_window << std::dec << ")\n";
    for (int i = 0; i < m_level; i++) stream << "\t";
    stream << "\tname: " << m_name << "\n";
    for (int i = 0; i < m_level; i++) stream << "\t";
    stream << "\tnofChilds: " << m_windowNofChilds;
    for (int i = 0; i < m_level; i++) stream << "\t";
    stream << "\tpid: " << m_pid;
    for (const auto& child : m_childs) {
        stream << std::string(child);
    }
    
    return stream.str();
}

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
                    spdlog::trace("Button event: {0} on {1}", buttonEvent->button, buttonEvent->subwindow);
                    WindowInfo wi = getWindowInfo(rootWindow);
                    // WindowInfo wi = getWindowInfo(buttonEvent->subwindow);
                    spdlog::trace("{0}", std::string(wi));
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

Window* X11Backend::getWindowChilds(const Window &window, unsigned int& resultLen)
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    Window rootWindow;
    Window parentWindow;
    Window* childWindows;
    if (! XQueryTree(m_display.get(), window, &rootWindow, &parentWindow, &childWindows, &resultLen)) {
        spdlog::error("Can't get window childs");
        resultLen = 0;
        return nullptr;
    }

    spdlog::trace("getWindowChilds(0x{0:x}): resultLen: {1}", window, resultLen);

    return childWindows;
}

unsigned char* X11Backend::getWindowProperty(const Window &window, const std::string& paramName, unsigned long& resultLen)
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    Atom prop = XInternAtom(m_display.get(), paramName.c_str(), False);
    Atom type;
    int actualFormatReturn;
    unsigned long bytesAfterReturn;
    unsigned char *propReturn;
    int status = XGetWindowProperty(m_display.get(), window, prop, 0, 1024, False,
                                    AnyPropertyType, &type,
                                    &actualFormatReturn, &resultLen, &bytesAfterReturn,
                                    &propReturn);
    
    spdlog::trace("getWindowInfo(0x{0:x}): property: \"{1}\", resultLen: {2}, bytesLeft: {3}", window, paramName, resultLen, bytesAfterReturn);
    
    if (status != Success || bytesAfterReturn > 0) {
        spdlog::error("Can't get window param for the one request");
        XFree(propReturn);
        return nullptr;
    }

    if (resultLen) {
        spdlog::trace("getWindowInfo(0x{0:x}): value for \"{1}\" is: {2}", window, paramName, propReturn);
    }
    
    return propReturn;
}

WindowInfo X11Backend::getWindowInfo(const Window &window, const int level)
{
    spdlog::trace("{0}({1})::{2}()", __FILE__, __LINE__, __func__);

    unsigned long namePropLen = 0;
    unsigned char* nameProp = getWindowProperty(window, "WM_NAME", namePropLen);

    unsigned long pidPropLen = 0;
    unsigned char* pidProp = getWindowProperty(window, "_NET_WM_PID", pidPropLen);

    WindowInfo info;
    info.m_window = window;
    info.m_level = level;

    if (namePropLen > 0) info.m_name = std::string(reinterpret_cast<char*>(nameProp));
    XFree(nameProp);
    int CARDINAL_SIZE = 4; // 32bit / 8
    if (pidPropLen > 0) std::memcpy(&info.m_pid, pidProp, CARDINAL_SIZE);
    XFree(pidProp);

    info.m_windowChilds = getWindowChilds(window, info.m_windowNofChilds);

    for (int i = 0; i < info.m_windowNofChilds; i++) {
        info.m_childs.push_back(getWindowInfo(info.m_windowChilds[i], level + 1));
    }

    return info;
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