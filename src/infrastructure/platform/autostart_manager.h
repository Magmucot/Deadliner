#pragma once

#include <QString>

namespace deadliner::infrastructure {

class AutostartManager {
public:
    bool isEnabled() const;
    bool setEnabled(bool enabled, const QString &applicationPath, const QString &arguments = QString()) const;

private:
    QString desktopFilePath() const;
};

}  // namespace deadliner::infrastructure
