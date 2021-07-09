#pragma once

#include "i_windows_backend.h"

#include <QObject>

namespace ui {

class MouseTrackerProxy : public QObject
{
    Q_OBJECT

public:
    void mouseGlobalPositionCallback(int x, int y);

signals:
    void mouseGlobalPositionChanged(int x, int y);
};

} // namespace ui
