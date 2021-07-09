#include "mouse_tracker_proxy.h"
#include "log.h"

namespace ui {

void MouseTrackerProxy::mouseGlobalPositionCallback(int x, int y)
{
    LOG_FUNC_ENTRY_MSG_ARGS("x: {1}, y: {2}", x, y);
    emit mouseGlobalPositionChanged(x, y);
}

} // namespace ui
