#pragma once

#include <QMetaType>

namespace deadliner::ui {

enum class MainSection {
    Today,
    Events,
    Overdue,
    Profiles,
    Statistics,
    Settings,
};

}  // namespace deadliner::ui

Q_DECLARE_METATYPE(deadliner::ui::MainSection)
