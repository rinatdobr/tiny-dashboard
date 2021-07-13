#include <QGuiApplication>
#include <QQuickView>
#include <QtQml>

#include "backend/i_window.h"
#include "backend/i_windows_backend.h"
#include "backend/x11/backend.h"
#include "ui/centre.h"
#include "ui/mouse_tracker_proxy.h"

#include <iostream>
#include <memory>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::info("Welcome to tiny-dashboard");

    // mouse tracker setup
    ui::MouseTrackerProxy mouseTrackerProxy;
    tdwindows::MouseTrackerCb mouseTrackerCb = [&mouseTrackerProxy](int x, int y) {
        mouseTrackerProxy.mouseGlobalPositionCallback(x, y);
    };

    // windows backend setup
    std::unique_ptr<tdwindows::IWindowsBackend> windowsBackend(new x11::Backend());
    windowsBackend->setMouseTrackerCallback(mouseTrackerCb);
    if (!windowsBackend->start()) {
        return EXIT_FAILURE;
    }

    // application window
    QScopedPointer<QGuiApplication> app(new QGuiApplication(argc, argv));
    QQmlApplicationEngine engine;
    engine.load(QUrl("qrc:/tiny-dashboard.qml"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    engine.rootContext()->setContextProperty(QStringLiteral("mouseTrackerProxy"),
                                             &mouseTrackerProxy);

    int rc = app->exec();
    windowsBackend->stop();
    spdlog::info("Bye-bye!");
    return rc;
}
