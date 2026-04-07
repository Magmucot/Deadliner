#pragma once

#include <QString>

namespace deadliner::infrastructure {

void initializeLogging(const QString &directoryPath);
QString logFilePath();

}  // namespace deadliner::infrastructure
