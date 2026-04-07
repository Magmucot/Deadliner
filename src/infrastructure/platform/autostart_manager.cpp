#include "infrastructure/platform/autostart_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

namespace deadliner::infrastructure {

bool AutostartManager::isEnabled() const
{
#ifdef Q_OS_WIN
    QSettings settings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);
    return settings.contains(QStringLiteral("Deadliner"));
#else
    return QFile::exists(desktopFilePath());
#endif
}

bool AutostartManager::setEnabled(bool enabled, const QString &applicationPath, const QString &arguments) const
{
#ifdef Q_OS_WIN
    QSettings settings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);
    if (enabled) {
        settings.setValue(QStringLiteral("Deadliner"), QStringLiteral("\"%1\" %2").arg(applicationPath, arguments));
    } else {
        settings.remove(QStringLiteral("Deadliner"));
    }
    return settings.status() == QSettings::NoError;
#else
    const QString path = desktopFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    if (!enabled) {
        return !QFile::exists(path) || QFile::remove(path);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=Deadliner\n";
    stream << "Exec=\"" << applicationPath << "\" " << arguments << "\n";
    stream << "X-GNOME-Autostart-enabled=true\n";
    stream << "Terminal=false\n";
    return true;
#endif
}

QString AutostartManager::desktopFilePath() const
{
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return QDir(configHome).filePath(QStringLiteral("autostart/deadliner.desktop"));
}

}  // namespace deadliner::infrastructure
