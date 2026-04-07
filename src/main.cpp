#include "app/app_controller.h"
#include "infrastructure/logging/logging.h"

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Deadliner"));
    QApplication::setOrganizationName(QStringLiteral("Deadliner"));
    QApplication::setQuitOnLastWindowClosed(false);
    app.setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    const QString logDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("logs"));
    deadliner::infrastructure::initializeLogging(logDir);

    deadliner::app::AppController controller;
    if (!controller.initialize()) {
        return 1;
    }

    return app.exec();
}
