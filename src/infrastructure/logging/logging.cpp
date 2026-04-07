#include "infrastructure/logging/logging.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <memory>

namespace deadliner::infrastructure {

namespace {

QMutex g_mutex;
QString g_path;
std::unique_ptr<QFile> g_file;

void fileMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &message)
{
    QMutexLocker locker(&g_mutex);
    if (!g_file || !g_file->isOpen()) {
        return;
    }

    const QString level = [type]() {
        switch (type) {
        case QtDebugMsg:
            return QStringLiteral("DEBUG");
        case QtInfoMsg:
            return QStringLiteral("INFO");
        case QtWarningMsg:
            return QStringLiteral("WARN");
        case QtCriticalMsg:
            return QStringLiteral("ERROR");
        case QtFatalMsg:
            return QStringLiteral("FATAL");
        }
        return QStringLiteral("INFO");
    }();

    QTextStream stream(g_file.get());
    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << ' '
           << level << ' ' << message << '\n';
    stream.flush();
}

}  // namespace

void initializeLogging(const QString &directoryPath)
{
    QDir().mkpath(directoryPath);
    g_path = QDir(directoryPath).filePath(QStringLiteral("deadliner.log"));
    g_file = std::make_unique<QFile>(g_path);
    if (g_file->open(QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler(fileMessageHandler);
    }
}

QString logFilePath()
{
    return g_path;
}

}  // namespace deadliner::infrastructure
