#pragma once

#include "i_windows_backend.h"

#include <QObject>

namespace ui {

class MouseTrackerProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x)
    Q_PROPERTY(int y READ y)

public:
    explicit MouseTrackerProxy(QObject *parent = nullptr);

    int x() const;
    int y() const;
    void mouseGlobalPositionCallback(int x, int y);

signals:
    void mouseGlobalPositionChanged(int x, int y);

private:
    int m_x = 0;
    int m_y = 0;
};

} // namespace ui
