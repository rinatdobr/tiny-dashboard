#include "mouse_tracker_proxy.h"
#include "log.h"

namespace ui {

MouseTrackerProxy::MouseTrackerProxy(QObject *parent)
{
    LOG_FUNC_ENTRY();
}

int MouseTrackerProxy::x() const
{
    LOG_FUNC_ENTRY();

    return m_x;
}

int MouseTrackerProxy::y() const
{
    LOG_FUNC_ENTRY();

    return m_y;
}

void MouseTrackerProxy::mouseGlobalPositionCallback(int x, int y)
{
    LOG_FUNC_ENTRY_MSG_ARGS("x: {1}, y: {2}", x, y);

    m_x = x;
    m_y = y;
    emit mouseGlobalPositionChanged(x, y);
}

} // namespace ui
