#include "backend.h"
#include "log.h"
#include "window.h"

#include <X11/Xatom.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace x11 {

Backend::Backend()
    : m_display(nullptr, XCloseDisplay)
{
    LOG_FUNC_ENTRY();

    setState(State::Ready);
}

Backend::~Backend()
{
    LOG_FUNC_ENTRY();

    stop();
}

bool Backend::start()
{
    LOG_FUNC_ENTRY();

    if (!isReady()) {
        spdlog::error("start: x11 backend is not ready");
        return false;
    }

    spdlog::info("X11 backend is starting...");

    std::unique_lock<std::mutex> _ul(m_workingThreadStartMutex);
    m_workingThread = std::thread(&Backend::workingThread, this);

    // waiting for thread start
    m_workingThreadStartNotifier.wait(_ul, [this] { return isInvalid() || isWorking(); });

    if (!isWorking()) {
        spdlog::error("start: x11 backend is not working");
        return false;
    }

    spdlog::info("X11 backend started");
    return true;
}

void Backend::stop()
{
    LOG_FUNC_ENTRY();

    if (!isWorking()) {
        spdlog::error("stop: x11 backend is not working");
        return;
    }

    for (auto &w : m_createdWindows) {
        spdlog::trace("w: {0}", w.use_count());
        XDestroyWindow(m_display.get(), w->getWindow());
        w.reset();
    }
    m_createdWindows.clear();

    if (isWorking()) {
        m_workingThreadFlag = false;
        sendDummyEvent();
        m_workingThread.join();
    }

    setState(State::Ready);

    spdlog::info("X11 backend stopped");
}

void Backend::workingThread()
{
    LOG_FUNC_ENTRY();

    spdlog::info("X11 backend working thread starting...");

    {
        std::lock_guard<std::mutex> _ul(m_workingThreadStartMutex);
        Status status = XInitThreads();
        if (!status) {
            spdlog::error("workingThread: can't init x11 threads");
            setState(State::Invalid);
            return;
        }

        m_display.reset(XOpenDisplay(NULL));
        if (!m_display) {
            spdlog::error("workingThread: can't open x11 display");
            setState(State::Invalid);
            return;
        }
        setState(State::Working);
        getDisplayInfo();
        m_workingThreadFlag = true;
    }
    m_workingThreadStartNotifier.notify_one();

    spdlog::info("Started X11 working thread");

    x11Window rootWindow = XDefaultRootWindow(m_display.get());
    unsigned int eventMask = ButtonPressMask | PointerMotionMask;
    XEvent event;

    // grab pointer
    if (XGrabPointer(m_display.get(),
                     rootWindow,
                     False,
                     eventMask,
                     GrabModeAsync,
                     GrabModeAsync,
                     None,
                     None,
                     CurrentTime)
        != GrabSuccess) {
        spdlog::error("workingThread: can't grab x11 pointer");
        setState(State::Invalid);
        m_display.reset();
        return;
    }

    // select events we are interested in
    XSelectInput(m_display.get(), rootWindow, eventMask);

    // handle events
    while (m_workingThreadFlag) {
        // get the next event
        XNextEvent(m_display.get(), &event);
        switch (event.type) {
        case ButtonPress: {
            XButtonEvent *buttonEvent = reinterpret_cast<XButtonEvent *>(&event);
            spdlog::trace("Button event: {0} on 0x{1:x}",
                          buttonEvent->button,
                          buttonEvent->subwindow);
            WindowInfo rootW = getWindowInfo(buttonEvent->root);
            spdlog::trace("rootW: {0}", std::string(rootW));
            WindowInfo wW = getWindowInfo(buttonEvent->window);
            spdlog::trace("wW: {0}", std::string(wW));
            WindowInfo subW = getWindowInfo(buttonEvent->subwindow);
            spdlog::trace("subW: {0}", std::string(subW));
        } break;
        case MotionNotify: {
            XMotionEvent *motionEvent = reinterpret_cast<XMotionEvent *>(&event);
            spdlog::trace("Motion event: x: {0}, y: {1}", motionEvent->x, motionEvent->y);
            // WindowInfo rootW = getWindowInfo(motionEvent->window);
            // spdlog::trace("root: {0}", std::string(rootW));
            // WindowInfo subW = getWindowInfo(motionEvent->subwindow);
            // spdlog::trace("subW: {0}", std::string(subW));
        } break;
        }
    }

    // return pointer
    XUngrabPointer(m_display.get(), CurrentTime);
    m_display.reset();

    spdlog::info("X11 backend working thread finished");
}

void Backend::getDisplayInfo()
{
    LOG_FUNC_ENTRY();

    int nofScreens = XScreenCount(m_display.get());

    spdlog::debug("Display info: ");
    spdlog::debug("\tNumber of screens: {0}", nofScreens);
    for (int i = 0; i < nofScreens; i++) {
        spdlog::debug("\tScreen info [{0}]", i);
        spdlog::debug("\t\twidth: {0}", XDisplayWidth(m_display.get(), i));
        spdlog::debug("\t\theight: {0}", XDisplayHeight(m_display.get(), i));
    }
}

void Backend::sendDummyEvent()
{
    LOG_FUNC_ENTRY();

    XEvent event = {};
    event.type = MotionNotify;
    event.xmotion.x = 0;
    event.xmotion.y = 0;
    event.xmotion.x_root = 0;
    event.xmotion.y_root = 0;
    event.xmotion.time = CurrentTime;
    event.xmotion.same_screen = True;

    if (!XSendEvent(m_display.get(),
                    XDefaultRootWindow(m_display.get()),
                    True,
                    PointerMotionMask,
                    &event)) {
        spdlog::error("sendDummyEvent: can't send x11 event");
        return;
    }

    XFlush(m_display.get());
}

std::vector<x11Window> Backend::getWindowChilds(const x11Window &window)
{
    LOG_FUNC_ENTRY();

    x11Window rootW;
    x11Window parentW;
    x11Window *childWs;
    unsigned int nofChildWs = 0;
    if (!XQueryTree(m_display.get(), window, &rootW, &parentW, &childWs, &nofChildWs)) {
        spdlog::error("getWindowChilds: can't get x11 window childs");
        nofChildWs = 0;
        return {};
    }

    std::vector<x11Window> childs(childWs, childWs + nofChildWs);
    spdlog::trace("getWindowChilds(0x{0:x}): nofChilds={1}", window, nofChildWs);
    XFree(childWs);

    return childs;
}

std::vector<char> Backend::getWindowProperty(const x11Window &window, const std::string &paramName)
{
    LOG_FUNC_ENTRY_MSG_ARGS("window=0x{1:x}, paramName={2}", window, paramName);

    Atom prop = XInternAtom(m_display.get(), paramName.c_str(), False);
    Atom actualAtomReturn;
    int actualFormatReturn;
    unsigned long bytesAfterReturn;
    unsigned long resultLen = 0;
    unsigned char *propReturn = nullptr;
    int status = XGetWindowProperty(m_display.get(),
                                    window,
                                    prop,
                                    0,
                                    1024,
                                    False,
                                    AnyPropertyType,
                                    &actualAtomReturn,
                                    &actualFormatReturn,
                                    &resultLen,
                                    &bytesAfterReturn,
                                    &propReturn);

    spdlog::trace("getWindowProperty:0x{0:x}: paramName={1}, actualAtomReturn={2}, "
                  "actualFormatReturn={3}, resultLen={4}, bytesAfterReturn={5}",
                  window,
                  paramName,
                  actualAtomReturn,
                  actualFormatReturn,
                  resultLen,
                  bytesAfterReturn);

    if (status != Success) {
        spdlog::error("getWindowProperty:0x{0:x}: can't get window property", window);
        return {};
    }

    std::vector<char> data(propReturn, propReturn + actualFormatReturn / 8 * resultLen);
    XFree(propReturn);

    if (bytesAfterReturn > 0) {
        spdlog::error("getWindowProperty:0x{0:x}: not all data had been read", window);
        return {};
    }

    if (!resultLen) {
        spdlog::error("getWindowProperty:0x{0:x}: no data read", window);
        return {};
    }

    return data;
}

WindowInfo Backend::getWindowInfo(const x11Window &window, const int level)
{
    LOG_FUNC_ENTRY_MSG_ARGS("window=0x{1:x}, level={2}", window, level);

    std::vector<char> name = getWindowProperty(window, "_NET_WM_NAME");
    std::vector<char> pid = getWindowProperty(window, "_NET_WM_PID");

    WindowInfo info = {};
    info.setX11Window(window);
    info.setLevel(level);
    info.setName(std::string(name.begin(), name.end()));
    pid_t pidBuf;
    if (sizeof(pidBuf) < pid.size()) {
        spdlog::error("getWindowInfo:0x{0:x}: pid mislengthing");
        return {};
    }
    std::copy(pid.data(), pid.data() + pid.size(), reinterpret_cast<char *>(&pidBuf));
    info.setPid(pidBuf);
    std::vector<WindowInfo> subWindowsInfo;
    std::vector<x11Window> subWindows = getWindowChilds(window);
    std::transform(subWindows.begin(),
                   subWindows.end(),
                   std::back_inserter(subWindowsInfo),
                   [this, level](x11Window window) { return getWindowInfo(window, level + 1); });
    info.setSubWindows(subWindowsInfo);

    return info;
}

void Backend::setState(const State &state)
{
    LOG_FUNC_ENTRY_MSG_ARGS("state={1}", m_State_str[static_cast<int>(state)]);

    static std::mutex _stateMutex;

    std::lock_guard<std::mutex> _stateLg(_stateMutex);
    m_state = state;
}

bool Backend::isInvalid()
{
    LOG_FUNC_ENTRY();

    return m_state == State::Invalid;
}

bool Backend::isReady()
{
    LOG_FUNC_ENTRY();

    return m_state == State::Ready;
}

bool Backend::isWorking()
{
    LOG_FUNC_ENTRY();

    return m_state == State::Working;
}

} // namespace x11
