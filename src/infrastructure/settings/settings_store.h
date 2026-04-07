#pragma once

#include "domain/models.h"

#include <QSettings>

namespace deadliner::infrastructure {

class SettingsStore {
public:
    SettingsStore();

    domain::AppSettings load() const;
    void save(const domain::AppSettings &settings) const;

private:
    mutable QSettings m_settings;
};

}  // namespace deadliner::infrastructure
